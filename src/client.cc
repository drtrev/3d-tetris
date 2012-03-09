/* 3d-tetris - A 3D multiuser Tetris game, originally made for researching collaborative interaction in virtual environments.
 *
 * Copyright (C) 2004-2011 Trevor Dodds <@gmail.com trev.dodds>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "client.h"
#include <cstdio>
#include <cstring>

// The client constructor
Client::Client()
{
  id = -1;
  receivedCursor = 0;
  overflow = false;
  readyToSend = 0;
  master = false;
  sendBlock = 0;
  oldNetworkData = "";
}

// Initialise server address struct and attempt connection to server
void Client::init()
{
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
    }

    serverAddress.sin_family = AF_INET;    // host byte order 
    serverAddress.sin_port = htons(PORT);  // short, network byte order 
    serverAddress.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(serverAddress.sin_zero), '\0', 8);  // zero the rest of the struct 

    //cout << "connecting .. " << endl;
    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1) {
      perror("connect");
      exit(1);
    }
    //cout << "connected!" << endl;
}

// Convert an int to a char... the old fashioned way (originally for testing purposes)
//   n - the number to convert
//
// Returns:
//   the character equivalent of n
char Client::intToChar(int n) {
  char c;
  switch (n)
  {
    case 0:
      c = '0'; break;
    case 1:
      c = '1'; break;
    case 2:
      c = '2'; break;
    case 3:
      c = '3'; break;
    case 4:
      c = '4'; break;
    case 5:
      c = '5'; break;
    case 6:
      c = '6'; break;
    case 7:
      c = '7'; break;
    case 8:
      c = '8'; break;
    case 9:
      c = '9'; break;
  }

  return c;
}

// Convert a char to an int
//   c - the char to convert
// 
// Returns:
//   the integer equivalent of c
int Client::charToInt(char c)
{
  int v = -1;
  
  switch (c) {
    case '0':
      v = 0; break;
    case '1':
      v = 1; break;
    case '2':
      v = 2; break;
    case '3':
      v = 3; break;
    case '4':
      v = 4; break;
    case '5':
      v = 5; break;
    case '6':
      v = 6; break;
    case '7':
      v = 7; break;
    case '8':
      v = 8; break;
    case '9':
      v = 9; break;
  }
      
  return v;
}

// Deal with adding zeros to the beginning of a number to make it
// an eight character string
//   num - the number to make an eight character string
//
// Returns:
//   an eight character string equivalent of num
string Client::dealZeros(float num)
{ // makes num an 8 character string
  string result = "";

  // 3 decimal places taken into account
  int n = (int) (num * 1000.0);

  // mark as negative 1 or positive 0
  if (n < 0){
    result = "1";
    n = -n;
  }else{
    result = "0";
  }

  int subtract; // subtract 1 digit at a time
  subtract = n / 1000000;
  n -= subtract * 1000000;
  result += intToChar(subtract);
  subtract = n / 100000;
  n -= subtract * 100000;
  result += intToChar(subtract);
  subtract = n / 10000;
  n -= subtract * 10000;
  result += intToChar(subtract);
  subtract = n / 1000;
  n -= subtract * 1000;
  result += intToChar(subtract);
  subtract = n / 100;
  n -= subtract * 100;
  result += intToChar(subtract);
  subtract = n / 10;
  n -= subtract * 10;
  result += intToChar(subtract);
  subtract = n;
  result += intToChar(subtract);

  return result;
}

// Create a new human (avatar)
//   id - the identification number to give the human
//   humans - the vector storing all humans in the system
//
// Returns:
//   the element number containing the new human
int Client::makeNewHuman(int id, vector <Human>& humans)
{
  cout << "A new user has been created!" << endl;

  // given humans an id of 100+
  int globalId = 100+humans.size();
  int texFace = 0;
  if (master) texFace = 2;
  else texFace = 3;
  humans.push_back(Human(0, 0, 0, 0, 0, texFace, globalId));
  humans[humans.size()-1].setSpeed(1.0); // TODO is this needed?
  humanIds.push_back(id);

  oldNetworkData = ""; // force resend

  return humanIds.size() - 1;
}

// Write some data into the send buffer for sending when the socket is writable
//   networkData - a string of data to send
void Client::sendData(string networkData)
{
  // 2 is STX (start of text)
  // 3 is ETX (end of text)
  networkData = (char) 2 + networkData + (char) 3;

  sendBuf.write(networkData.c_str());
}

// Transmit data that has been buffered for sending
void Client::transmitBufferedData()
{
  // TODO combine this select() with the read one?
  // declare numReadable sockets
  int numReadable = 0;
  int amountToRead; // amount to read from sendBuf
  int amountOfDataRead = 0; // amount read from sendBuf

  // zero time out - don't sit there waiting we're working in real time!
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  FD_ZERO(&writeSocks);
  FD_SET(sockfd, &writeSocks); // check for sockfd being writable

  // what's the status of the socket? is it writable?
  numReadable = select(sockfd + 1, NULL, &writeSocks, NULL, &timeout);

  // something went wrong
  if (numReadable == -1){
    perror("select");
    cout << "select error" << endl;
  }

  // we've got a writeable socket!
  if (numReadable > 0) {
    // read the data from buffer
    amountToRead = sendBuf.getDataOnBuffer();
    char buffer[amountToRead];
    amountOfDataRead = sendBuf.read(buffer, amountToRead);
    
    //cout << "sending: ";
    //for (int i = 0; i < amountOfDataRead; i++) cout << buffer[i];
    //cout << endl;

    // send with error check
    if (send(sockfd, buffer, amountOfDataRead, 0) == -1) {
      perror("send");
      cout << "send error" << endl;
    }
  }
}

// Manipulate a block based on a received manipulation data unit
//   blocks - a vector storing all the game blocks
//   blockNum - the number of the block to be manipulated
//   axis - the axis to rotate the block about
//   direction - the direction in which rotation should take place
void Client::manipulateBlock(vector <Block> &blocks, int blockNum, char axis, int direction)
{
  if (blockNum < 0 || blockNum > (int) blocks.size() - 1) {
    cerr << "Client::manipulateBlock(): blockNum out of range: " << blockNum << endl;
  }else{
    bool turningOrMoving = false;

    if ((blocks[blockNum].getTurning() || blocks[blockNum].getMoving())) turningOrMoving = true;

    // if block is being moved or turned (by you), and not master, then hit and move (let master take priority)
    if (turningOrMoving && !blocks[blockNum].getRemoteMovement() && !master) {
      blocks[blockNum].hit();
    }

    // block is not turningOrMoving -- go ahead
    // block is turningOrMoving by you and you're !master -- go ahead (will have hit above)
    // block is turning or moving by remote -- add to target position
    if (!turningOrMoving || (!blocks[blockNum].getRemoteMovement() && !master) || blocks[blockNum].getRemoteMovement()) {
      float newAngle = direction * 180 - 90; // -90 or 90

      // narrow down condition
      // first adds to target, second sets target
      if (turningOrMoving && blocks[blockNum].getRemoteMovement()) {
        // buffer the movement for later
        char temp[4];
        temp[0] = blockNum + '0';
        temp[1] = axis + '0';
        temp[2] = direction + '0';
        temp[3] = '\0';
        manipBuf.write(temp);
      }else{
        if (axis == 'X') blocks[blockNum].setTargetAngleX(newAngle);
        if (axis == 'Y') blocks[blockNum].setTargetAngleY(newAngle);
        if (axis == 'Z') blocks[blockNum].setTargetAngleZ(newAngle);
      }

      blocks[blockNum].unGrounded();
      
      // unground anything above this block
      for (int i = 0; i < (int) blocks.size(); i++) {
        if (blocks[i].getY() > blocks[blockNum].getY()) blocks[i].unGrounded();
      }
    }

  }
}

// Move a block based on a received data unit
//   blocks - the vector storing all the game blocks
//   blockNum - the number of the block to be moved
//   direction - the direction in which to move the game block
void Client::moveBlock(vector <Block> &blocks, int blockNum, int direction)
{
  if (blockNum < 0 || blockNum > (int) blocks.size() - 1) {
    cerr << "Client::moveBlock(): blockNum out of range: " << blockNum << endl;
  }else{
    float moveBlockX = 0.0, moveBlockY = 0.0, moveBlockZ = 0.0;
    switch (direction) {
      case 0:
        moveBlockX = -5.0;
        break;
      case 1:
        moveBlockY = -5.0;
        break;
      case 2:
        moveBlockZ = -5.0;
        break;
      case 3:
        moveBlockX = 5.0;
        break;
      case 4:
        moveBlockY = 5.0;
        break;
      case 5:
        moveBlockZ = 5.0;
        break;
      default:
        cerr << "Client::moveBlock(): direction not recognised: " << direction << endl;
    }

    bool turningOrMoving = false;

    if ((blocks[blockNum].getTurning() || blocks[blockNum].getMoving())) turningOrMoving = true;

    // if block is already turning (by you) and you're not the master then allow received data priority
    if (turningOrMoving && !blocks[blockNum].getRemoteMovement() && !master){
      blocks[blockNum].hit();
    }

    // block is not turningOrMoving -- go ahead
    // block is turningOrMoving by you and you're !master -- go ahead (will have hit above)
    // block is turning or moving by remote -- add to target position
    if (!turningOrMoving || (!blocks[blockNum].getRemoteMovement() && !master) || blocks[blockNum].getRemoteMovement()) {
     
      // narrow down condition
      // first adds to target, second adds to position
      if (turningOrMoving && blocks[blockNum].getRemoteMovement()) {
        char temp[3];
        temp[0] = blockNum + '0';
        temp[1] = direction + '0';
        temp[2] = '\0';
        moveBuf.write(temp);
      }else{
        blocks[blockNum].setRemoteMovement(true);
        // setTargetPosition sets oldX,Y,Z for collision purposes, changeTargetPosition doesn't do that
        blocks[blockNum].setTargetPosition(blocks[blockNum].getX() + moveBlockX,
            blocks[blockNum].getY() + moveBlockY, blocks[blockNum].getZ() + moveBlockZ);
      }

      blocks[blockNum].unGrounded();
      
      // unground anything above this block
      for (int i = 0; i < (int) blocks.size(); i++) {
        if (blocks[i].getY() > blocks[blockNum].getY()) blocks[i].unGrounded();
      }
    }

  }

}

// Called to do all client operations
//   player - the player object
//   humans - the humans inhabiting the CVE
//   blocks - the blocks within the CVE
//   blockType - the type of a block to be created
//   gravity - a boolean to initiate gravity
//   receivedLock - a boolean indicating that a lock has been received
//   receivedRemoveLayer - a boolean indicating that a 'remove layer' request has been received
void Client::doClient(Pawn& player, vector <Human>& humans, vector <Block> &blocks, int &blockType, bool &gravity, bool &receivedLock, vector <int> &receivedRemoveLayer)
{
  // client stuff
  int numReadable = 0; // number of sockets readable
  char buf[MAXRECVDATASIZE];
  int lockNumber = 0; // which block is to be locked?

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  FD_ZERO(&readSocks);
  FD_SET(sockfd, &readSocks); // check for sockfd

  numReadable = select(sockfd + 1, &readSocks, NULL, NULL, &timeout);

  if (numReadable == -1){
    perror("select");
    cout << "select error" << endl;
  }

  if (numReadable > 0) { // there's data to read
    if ((numbytes=recv(sockfd, buf, MAXRECVDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    if (numbytes > MAXRECVDATASIZE - 1)
      cout << "warning: " << numbytes << " received in one go - bigger than MAXRECVDATASIZE!" << endl;

    //cout << "received: ";
    //for (int i = 0; i < numbytes; i++) cout << buf[i];
    //cout << endl;

    // first char is id, then the rest is position data
    // for the first recvd though, the client is assigned an id
    // (an 'i' followed by their id)
    //buf[numbytes] = '\0'; // for outputting to screen, make like a string

    // start reading from the beginning
    int startIndex = 0;
    int originalIndex = 0; // store so we can go back to this if we don't have a full chunk

    // id assignment is first thing sent (not start of data char, that's sent by doServer)
    if (id == -1) {
      if (buf[0] == 'i') { // this should be first
        startIndex = 1;
        id = (int) buf[startIndex++]; // mark first char as read
        cout << "I have been assigned an id of: " << id << endl;
        if (buf[startIndex++] == FLAG_MASTER) {
          master = true;
          cout << "I am the master (primary client)!" << endl;
        }else{
          master = false;
          cout << "I am the slave (secondary client)!" << endl;
        }
       //cout << "full buffer: " << buf << endl;
      }else{
        cout << "still waiting for id" << endl;
      }
    }
    
    if (id > -1) { // we've got our id (i.e. not still waiting for it)

      // now let's append anything else to the permanent (global) buffer
      for (int i = startIndex; i < numbytes; i++) {
        receivedSoFar[receivedCursor++] = buf[i];
        if (receivedCursor > MAXRECVDATASIZE - 1) {
          cout << "permanent store receivedSoFar was overflowed: reset took place" << endl;
          receivedCursor = 0;
          numbytes = 0;
          overflow = true;
        }
      }

      // the id of the data we are reading, and the last bit of data we read
      int dataId = -1, value = 0, humanId = -1, chunk = 0;
      char flag = FLAG_NONE;
      bool negative = false;
      vector <float> data; // a temporary store of the data

      // it's read in something but this may not be complete until next loop
      // e.g. x position might be sent, but y and z would have to wait
      // or also half a position may be sent!

      // so have we got a full data chunk that can be read?
      // a full chunk (for position): 40 chars + id + flag, receivedCursor will be 42
      // or just 40 chars received Cursor will be 40 above startIndex (chars 0-39)
      // startIndex set as 0 above and may be more than 0 if it's read in id

      // read in 2 indicating start of data
      // this should be first char - if not just find a 2
      // commented 7/2/04
      for (int i = 0; i < receivedCursor; i++) {
        if (receivedSoFar[i] == 2) {
          startIndex = i+1;
          originalIndex = i;
          if (overflow) cout << "overflow recovered" << endl;
          overflow = false; // ok to read data now- we're back in business
          break; // we've found a two - don't want to find another!
        }
      }

      // flag new block is followed by block type
      // flag gravity has no data following it
      if (receivedCursor > startIndex + 1) { // NOTE this is repeated below
        //cout << "in block thing" << endl;
        dataId = (int) receivedSoFar[startIndex++];
        flag = receivedSoFar[startIndex++];
      }

      chunk = receivedCursor - startIndex;

      // if while loop will fail, go back to original index so STX is not lost
      // IF ALTERING CHUNK SIZE DO IT IN THE WHILE LOOP TOO!
      if (!(((flag == FLAG_POSITION && chunk > 39) || (flag == FLAG_LOCK && chunk > 0) || (flag == FLAG_MANIPULATE && chunk > 2)
             || (flag == FLAG_NEW_BLOCK && chunk > 0) || (flag == FLAG_MOVE && chunk > 1) || (flag == FLAG_GRAVITY && chunk > -1)
             || (flag == FLAG_BLOCK && chunk > 19 * 8 + 1) || (flag == FLAG_LAYER_FOUND && chunk > 0) || (flag == FLAG_LAYER_REMOVE && chunk > 0))
             && !overflow)) {
        // since we've not got enough data, the while loop will not execute (flag is FLAG_NONE)
        // and the array will be shifted back based on startIndex. But we want the STX (2) character
        // to be read again next time, so...
        startIndex = originalIndex;
      }

      // check we've got enough data for a full chunk
      // also can't process data if we have an overflow
      // do add a new flag we need to add a condition here
      // IF ALTERING CHUNK SIZE DO IT IN THE ABOVE CONDITION TOO!
      while (((flag == FLAG_POSITION && chunk > 39) || (flag == FLAG_LOCK && chunk > 0) || (flag == FLAG_MANIPULATE && chunk > 2)
             || (flag == FLAG_NEW_BLOCK && chunk > 0) || (flag == FLAG_MOVE && chunk > 1) || (flag == FLAG_GRAVITY && chunk > -1)
             || (flag == FLAG_BLOCK && chunk > 19 * 8 + 1) || (flag == FLAG_LAYER_FOUND && chunk > 0)
             || (flag == FLAG_LAYER_REMOVE && chunk > 0)) && !overflow) {

        // if it's not you
        if (id != dataId) {

          // which human has dataId?
          humanId = -1;
          for (unsigned int i = 0; i < humanIds.size(); i++) {
            if (dataId == humanIds[i]) humanId = i;
          }
          if (humanId == -1 && dataId != SERVER_ID) humanId = makeNewHuman(dataId, humans);
          
          // note humanId will still be -1 if not found or dataId is server ID
          // so just make sure humanId is not -1 still in any condition that makes use of humans
          // that way it accounts for dodgy server messages

          if (flag == FLAG_POSITION && humanId > -1) {
            // interpret the data
            negative = false;
            value = 0;
            data.clear();

            // read in main chunk of data - if size of chunk alters then alter 'startIndex +=' below
            for (int i = startIndex; i < startIndex+40; i+=8) {
              if (receivedSoFar[i] == '1') negative = true;
              else negative = false;

              value = 1000000 * charToInt(receivedSoFar[i+1]);
              value += 100000 * charToInt(receivedSoFar[i+2]);
              value += 10000 * charToInt(receivedSoFar[i+3]);
              value += 1000 * charToInt(receivedSoFar[i+4]);
              value += 100 * charToInt(receivedSoFar[i+5]);
              value += 10 * charToInt(receivedSoFar[i+6]);
              value += charToInt(receivedSoFar[i+7]);
              if (negative) value = -value;
              //cout << "value: " << value << endl;
              data.push_back(value / 1000.0);
            }

            // if it's not your data, it must be another humans
            //cout << "setting stuff: x: " << data[0] << " y: " << data[1] << " z: " << data[2] << endl;

            // stuff the data into corresponding human attributes
            // set target coordinates
            humans[humanId].setTargetX(data[0]);
            humans[humanId].setTargetY(data[1]);
            humans[humanId].setTargetZ(data[2]);

            // note - does not turn towards this target (X) yet
            //if (data[3] > -720 && data[3] < 720) humans[humanId].setTargetAngleX(data[3]);
            // set target angles
            if (data[3] > -720 && data[3] < 720) humans[humanId].setTargetAngleX(data[3]);
            if (data[4] > -720 && data[4] < 720) humans[humanId].setTargetAngleY(data[4]);

            //humans[0].setSpeed(data[4]); this seemed to mean human would go too far and jump back
            //humans[0].setSpeed(data[4]);

            startIndex += 40; // move the cursor to the next unread piece of data

          }else if (flag == FLAG_LOCK && humanId > -1) {
            //cout << "flag lock" << endl;
            // get block number to lock
            lockNumber = (int) (receivedSoFar[startIndex++] - '0'); // advance startIndex
            //if (lockNumber > blocks.size() - 1 || lockNumber < 0) {
            //  cerr << "client::doClient: lockNumber out of range: " << lockNumber << endl;
            //}else{
            humans[humanId].setLocked(lockNumber);
            receivedLock = true;
            //}
          }else if (flag == FLAG_NEW_BLOCK) {
            blockType = (int) (receivedSoFar[startIndex++] - '0'); // advance startIndex
            //cout << "received new block" << endl;
            // new block is handled by cve because it's the same method as single player
          }else if (flag == FLAG_GRAVITY) {
            gravity = true;
          }else if (flag == FLAG_MANIPULATE) {
            manipulateBlock(blocks, receivedSoFar[startIndex] - '0', receivedSoFar[startIndex+1], receivedSoFar[startIndex+2] - '0');
            startIndex += 3;
          }else if (flag == FLAG_MOVE) {
            moveBlock(blocks, receivedSoFar[startIndex] - '0', receivedSoFar[startIndex+1] - '0');
            startIndex += 2;
          }else if (flag == FLAG_BLOCK) {
            // interpret the data
            negative = false;
            value = 0;
            data.clear();

            // we know the id of the block in question, but which element of
            // the array is this?
            int blockId = (int) (receivedSoFar[startIndex++] - '0');
            int blockNum = -1;
            for (int i = 0; i < (int) blocks.size(); i++) {
              if (blocks[i].getId() == blockId) blockNum = i;
            }

            // does block exist? array may have changed due to a layer being
            // completed
            if (blockNum > -1) {
              // read in main chunk of data - if size of chunk alters then alter 'startIndex +=' below
              for (int i = startIndex; i < startIndex+19*8; i+=8) {
                if (receivedSoFar[i] == '1') negative = true;
                else negative = false;

                value = 1000000 * charToInt(receivedSoFar[i+1]);
                value += 100000 * charToInt(receivedSoFar[i+2]);
                value += 10000 * charToInt(receivedSoFar[i+3]);
                value += 1000 * charToInt(receivedSoFar[i+4]);
                value += 100 * charToInt(receivedSoFar[i+5]);
                value += 10 * charToInt(receivedSoFar[i+6]);
                value += charToInt(receivedSoFar[i+7]);
                if (negative) value = -value;
                //cout << "value: " << value << endl;
                data.push_back(value / 1000.0);
              }
              bool same = true;
              for (int i = 0; i < 16; i++) {
                if (blocks[blockNum].getConfirmedMatrix(i) < data[i] - 0.1 || blocks[blockNum].getConfirmedMatrix(i) > data[i] + 0.1) same = false;
                blocks[blockNum].setConfirmedMatrix(i, data[i]);
              }
              if (blocks[blockNum].getConfirmedX() < data[16] - 0.1 || blocks[blockNum].getConfirmedX() > data[16] + 0.1) same = false;
              if (blocks[blockNum].getConfirmedY() < data[17] - 0.1 || blocks[blockNum].getConfirmedY() > data[17] + 0.1) same = false;
              if (blocks[blockNum].getConfirmedZ() < data[18] - 0.1 || blocks[blockNum].getConfirmedZ() > data[18] + 0.1) same = false;
              blocks[blockNum].setConfirmedX(data[16]);
              blocks[blockNum].setConfirmedY(data[17]);
              blocks[blockNum].setConfirmedZ(data[18]);
              if (same) {
                blocks[blockNum].setGotConfirmed(true);
                //cout << "same" << endl;
              }
              startIndex += 19 * 8 + 1;
            }
          }else if (flag == FLAG_LAYER_REMOVE) {
            receivedRemoveLayer.push_back((int) receivedSoFar[startIndex++] - 1);
            cout << "received remove layer data unit: " << receivedRemoveLayer[(int) receivedRemoveLayer.size()-1] << endl;
          }else if (flag == FLAG_LAYER_FOUND) {
            cout << "received flag layer found... shouldn't have" << endl;
            startIndex++;
            // it's for the server only
          }else{
            cout << "flag was not found: " << flag << endl;
            cout << "in int format: " << (int) flag << endl;
          }
        } // end if it's not you
        else{
          cout << "WARNING: data is not for me - I should not have been sent this" << endl;
          // advance startIndex depending upon flag (data size)
          switch (flag) {
            case FLAG_POSITION:
              startIndex += 40; // move the cursor to the next unread piece of data
              break;
            case FLAG_LOCK:
              startIndex ++;
              break;
            case FLAG_MANIPULATE:
              startIndex += 3; // block number, axis, direction
              break;
            case FLAG_MOVE:
              // only move on remote instruction so any bugs affect both clients!!
              moveBlock(blocks, receivedSoFar[startIndex] - '0', receivedSoFar[startIndex+1] - '0');
              startIndex += 2; // block number, direction
              break;
            case FLAG_NEW_BLOCK:
              cerr << "error: got a server message for new block but dataId was same as me" << endl;
              startIndex ++;
              break;
            case FLAG_BLOCK:
              startIndex += 19 * 8 + 1;
              break;
            case FLAG_GRAVITY:
              cerr << "error: got a server message for gravity but dataId was same as me" << endl;
              break;
            case FLAG_LAYER_REMOVE:
              startIndex ++;
              break;
            case FLAG_LAYER_FOUND:
              startIndex ++;
              break;
          }
        }

        if (receivedCursor > startIndex + 2) { // TODO remove couts here
          //cout << "more data" << endl;
          if (receivedSoFar[startIndex] == 2) startIndex++; // there should be a STX here
          dataId = (int) receivedSoFar[startIndex++];
          flag = receivedSoFar[startIndex++];
          //if (flag == FLAG_LOCK) cout << "got a flag lock!!" << endl;
          //cout << "flag: " << flag << endl;
          //if (dataId == '\n') cout << "ho!" << endl;
          chunk = receivedCursor - startIndex;
        }else{
          flag = FLAG_NONE; // will exit while loop
        }
      } // end while there's still data to be processed

      // we now have startIndex which is the next character to be read from
      // receivedSoFar, and receivedCursor which is the next character index
      // to be written to in the receivedSoFar array
      // so copy all the stuff not yet used back to the beginning
      //
      // shift it all back to the start of the array
      receivedCursor -= startIndex;
      for (int i = 0; i < receivedCursor; i++) {
        receivedSoFar[i] = receivedSoFar[i+startIndex];
      }

    } // end if we've got our id

  } // end if there's data to read

  for (unsigned int i = 0; i < humans.size(); i++) {
    humans[i].move();
  }

  if (readyToSend) {
    readyToSend = 0;

    string networkData = FLAG_POSITION + dealZeros(player.getX()) + dealZeros(player.getY()) + dealZeros(player.getZ())
                         + dealZeros(player.getAngleX()) + dealZeros(player.getAngleY());

    if (networkData != oldNetworkData) {
      sendData(networkData); // adds 2 and 3 STX and ETX
      oldNetworkData = networkData;
    }

    if (sendBlock < (int) blocks.size()) {
      if (!blocks[sendBlock].getMoving() && !blocks[sendBlock].getTurning() && master) { // && blocks[sendBlock].getGrounded()) 
        networkData = "";
        networkData += FLAG_BLOCK;
        // we want to go through all blocks available and send them (sendBlock)
        // but because blocks may be split (and the vector change) the receiver needs to identify blocks
        // by their global id number not their element number
        networkData += (blocks[sendBlock].getId() + '0');
        for (int i = 0; i < 16; i++)
          networkData += dealZeros(blocks[sendBlock].getMatrix(i));
        networkData += dealZeros(blocks[sendBlock].getX());
        networkData += dealZeros(blocks[sendBlock].getY());
        //cout << "sending confirmedY: " << blocks[sendBlock].getY() << ", as dealZeros: " << dealZeros(blocks[sendBlock].getY()) << endl;
        networkData += dealZeros(blocks[sendBlock].getZ());

        sendData(networkData);

      }
      sendBlock++;
      if (sendBlock > (int) blocks.size() - 1) sendBlock = 0;
    }
    
  }

  // if there is data waiting to be sent, then attempt transmission
  if (sendBuf.getDataOnBuffer() > 0) transmitBufferedData();
  if (moveBuf.getDataOnBuffer() > 1) {
    // read the data from buffer
    char tempBlockNum;
    int tempBlockNumInt;

    // look at next character on buffer
    if (moveBuf.getChar(tempBlockNum, 0)) {

      tempBlockNumInt = (int) (tempBlockNum - '0');

      // we've got the block number, so if it's not turning or moving then read off the buffer and move it!
      if (tempBlockNumInt > (int) blocks.size() -1 || tempBlockNumInt < 0) {
        cerr << "client::doClient(): tempBlockNumInt out of range for moving blocks: " << tempBlockNumInt << endl;
      }else if (!blocks[tempBlockNumInt].getMoving() && !blocks[tempBlockNumInt].getTurning()) {
        char c[2];
        if (moveBuf.read(c, 2) == 2){
          moveBlock(blocks, (int) (c[0] - '0'), (int) (c[1] - '0'));
        }
      }
    }
  }
  if (manipBuf.getDataOnBuffer() > 2) {
    // read the data from buffer
    char tempBlockNum;
    int tempBlockNumInt;

    // look at next character on buffer
    if (manipBuf.getChar(tempBlockNum, 0)) {

      tempBlockNumInt = (int) (tempBlockNum - '0');

      // we've got the block number, so if it's not turning or moving then read off the buffer and manip it!
      if (tempBlockNumInt > (int) blocks.size() -1 || tempBlockNumInt < 0) {
        cerr << "client::doClient(): tempBlockNumInt out of range for manipulating blocks: " << tempBlockNumInt << endl;
      }else if (!blocks[tempBlockNumInt].getMoving() && !blocks[tempBlockNumInt].getTurning()) {
        char c[3];
        if (manipBuf.read(c, 3) == 3){
          manipulateBlock(blocks, (int) (c[0] - '0'), (int) (c[1] - '0'), (int) (c[2] - '0'));
        }
      }
    }
  }

}

// Selector method
//
// Returns:
//   a boolean value representing whether the client is master (true) or slave (false)
bool Client::getMaster()
{
  return master;
}

// Set the host to connect to
//   ip - the ip address or name of the host
void Client::setHost(const char* ip)
{
  host = gethostbyname(ip);
}

// Set the client's status as ready to send data
//   value - the value to assign to readyToSend (1 = ready, value used as an int due to glutTimerFunc())
void Client::setReadyToSend(int value)
{
  readyToSend = value;
}

// Close the connection with the server
//
// Returns:
//   int - return value from connection close operation
int Client::closeConnection()
{
  return close(sockfd);
}
