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

/*
 * server.cc
 *
 * A server program for the CVE.
 * Accepts new connections, passes on messages and sends out its own
 * messages for initialising gravity and creating new blocks.
 *
 * Trevor Dodds, 2005
 */

#include <vector>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "game.h"

using namespace std;

// Do we want to output information to the standard out, for logfiles?
#define LOG_OUTPUT 0

// For evaluation purposes set the NEW_BLOCK_COUNT_DECREASE to 0.2
// Then reset the server between games.
// Note that client log files are only created when the game over animation is
// complete.

#define MYPORT 3490    // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXRECVDATASIZE 1000

#define MAXCLIENTS 10
#define NUM_PLAYERS 2 // number of clients before starting gravity

#define MAX_SERVER_MESSAGE_SIZE 50

// define data unit flags
#define FLAG_MASTER 'M'
#define FLAG_SLAVE 'S'
#define FLAG_NEW_BLOCK 'n'
#define FLAG_GRAVITY 'g'
#define FLAG_GAMEOVER 'G'
#define FLAG_LAYER_FOUND 'L'
#define FLAG_LAYER_REMOVE 'R'

#define LAYER_FOUND_EXPIRY_TIME 40

// difficulty changing parameters (in seconds)
//
// NEW_BLOCK_COUNT_START, NEW_BLOCK_COUNT_MIN and NEW_BLOCK_COUNT_DECREASE
// now defined in game.h (RAR)
//

// the number interval for traffic output in seconds
#define TRAFFIC_ANALYSIS_COUNT 1

float newBlockCount = (float) (NEW_BLOCK_COUNT_START / 1000);

#define SERVER_ID 's'

int listenSock, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
char buf[MAXRECVDATASIZE]; // for receiving data
int receivedCursor[MAXCLIENTS];
char dataReceived[MAXCLIENTS][MAXRECVDATASIZE+1]; // add on id
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr; // connector's address information
int sin_size;
//struct sigaction sa;
int sockoptyes=1;
int numHosts=0;

char serverMessage[MAX_SERVER_MESSAGE_SIZE];
int messNum = 0;

bool assignedMaster = false, startedGravity = false;

int clCount = 0, clMax = 10; // client counter (how many clients we have) and maximum number
int clSocks[10]; // client sockets
int maxSock = 0;
struct timeval timeout;
fd_set readSocks;
fd_set writeSocks;

clock_t newBlockTimer; // for making new blocks appear
clock_t trafficAnalysisTimer; // for analysing traffic
int traffic; // number of bytes received

// store layer found details
vector <int> layerFound;
vector <int> timeLayerFound;
clock_t serverStartTime;
//int numBlocksSent = 0;

// Create a new block by generating a server message
void newBlock()
{
  //if (numBlocksSent > 1) return;
  //numBlocksSent++;

  int type = rand() % 8;

  serverMessage[0] = 2;   // STX
  serverMessage[1] = SERVER_ID; // id
  serverMessage[2] = FLAG_NEW_BLOCK;
  serverMessage[3] = type + '0'; // never send null
  //serverMessage[4] = 3;   // ETX no need to send this

  if (newBlockCount > (float) (NEW_BLOCK_COUNT_MIN / 1000))
    newBlockCount -= (float) (NEW_BLOCK_COUNT_DECREASE / 1000);
  if (LOG_OUTPUT) cerr << "newBlockCount: " << newBlockCount << endl;
  newBlockTimer = clock() + (long int) (newBlockCount * CLOCKS_PER_SEC);
  //cout << "new block info ready to send: " << type << endl;
}

// Output traffic for analysis
void analyseTraffic()
{
  if (LOG_OUTPUT) cout << traffic << endl;
  
  // reset traffic count
  traffic = 0;

  // reset timer
  trafficAnalysisTimer = clock() + (long int) (TRAFFIC_ANALYSIS_COUNT * CLOCKS_PER_SEC);
}

// The number of seconds elapsed since the server started
//
// Returns:
//   an integer representing the number of seconds elapsed since the server started
int secondsElapsed()
{
  return (int) ((clock() - serverStartTime) / CLOCKS_PER_SEC);
}

// Initialise server
void serverInit()
{
  serverStartTime = clock();

  // clear the data
  for (int i = 0; i < MAXCLIENTS; i++){
    receivedCursor[i] = 0;
    for (int j = 0; j < MAXRECVDATASIZE; j++) {
      dataReceived[i][j] = '\0';
    }
  }

  for (int i = 0; i < MAX_SERVER_MESSAGE_SIZE; i++) {
    serverMessage[i] = '\0';
  }

  // set random seed based on time
  srand( time(NULL) );

  if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &sockoptyes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_port = htons(MYPORT);     // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

  if (bind(listenSock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  if (listen(listenSock, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  newBlockTimer = clock() +
    (long int) ((float) (NEW_BLOCK_COUNT_START / 1000) * CLOCKS_PER_SEC);
  trafficAnalysisTimer = clock() + (long int) (TRAFFIC_ANALYSIS_COUNT * CLOCKS_PER_SEC);
}

// Receive data from clients
void receiveData()
{

  char id;

  // go through all the sockets and store the data
  for (int i = 0; i < clCount; i++) {
    
    // is the socket set as readable?
    if (FD_ISSET(clSocks[i], &readSocks)) {
      // receive a message
      if ((numbytes=recv(clSocks[i], buf, MAXRECVDATASIZE-1, 0)) == -1) {
          perror("recv");
          cerr << "receive error" << endl;
      }

      traffic += numbytes;

      // -1 is an error, dealt with above.
      // 0 means connection has been closed by remote side
      if (numbytes == 0){
        close(clSocks[i]);       
        cerr << "a connection was closed" << endl;
        clSocks[i] = -1; // it couldn't be this before, because that would mean
        // accept returned an error

        // clSocks[i] is removed from the array below
      }else{

        // store
        //buf[numbytes] = '\0'; // make like a string
        //printf("received buffer: %s\n", buf);
        //cout << "id assigning: " << clSocks[i] << " which goes to char: " << (char) clSocks[i] << endl;

        id = (char) (clSocks[i] % 256);

        // append received data to permanent store
        for (int j = 0; j < numbytes; j++) {

          // if char is 2 (start of text), then add id next to show who it was from
          dataReceived[i][receivedCursor[i]++] = buf[j];

          if (buf[j] == 2) dataReceived[i][receivedCursor[i]++] = id;

          if (receivedCursor[i] > MAXRECVDATASIZE - 2) { // -2 because 2 can be added at once from above
            cerr << "warning: permanent store dataReceived[" << i << "] was overflowed: reset took place" << endl;
            receivedCursor[i] = 0;
            numbytes = 0;
            //overflow = true;
          }
        
        }

      } // end if numbytes == 0

    } // end is socket readable

  } // end for


  // get rid of closed ones - note that this may not work
  // as they may not be readable by select
  // this would get rid of them if they were closed between select
  // and recv
  for (int i = 0; i < clCount; i++) {
    if (clSocks[i] == -1) {
      cerr << "removed connection from clSocks[]" << endl;
      for (int j = i; j < clCount-1; j++) {
        clSocks[j] = clSocks[j+1];
        for (int k = 0; k < MAXRECVDATASIZE; k++) dataReceived[j][k] = dataReceived[j+1][k];
        receivedCursor[j] = receivedCursor[j+1];
      }
      clCount--;
    }
  }

}

// Send some data
void sendData()
{
  // note this function should only be called if all sockets are writable
  int startOfData = -1, endOfData = -1;
  char dataToSend[MAXRECVDATASIZE];
  
  // alright we've got some data, so throw it out
  // go through data
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (dataReceived[i][0] != '\0') { // if there's data

      startOfData = -1, endOfData = -1;

      // is there a complete data chunk?
      // check for start and end characters 2 and 3
      for (int j = 0; j < receivedCursor[i]; j++) {
        if (dataReceived[i][j] == 2) {
          startOfData = j;
          for (int k = j+1; k < receivedCursor[i]; k++) {
            if (k == 0) cerr << "error" << endl;
            if (dataReceived[i][k] == 3) {
              endOfData = k;
              break;
            }
          }
          break;
        }
      } // end for

      // so can we send yet?
      if (startOfData > -1 && endOfData > -1) {
        //cout << "about to send: start: " << startOfData << " end: " << endOfData << " rc: " << receivedCursor[i] << endl;
        // random check
        if (endOfData <= startOfData) {
          cerr << "hold on, startOfData: " << startOfData << ", endOfData: " << endOfData
               << ", I thought I fixed this by initialising vars each loop" << endl;
          // nuke it
          for (int j = 0; j < MAXRECVDATASIZE; j++) dataReceived[i][j] = '\0';
          receivedCursor[i] = 0;
          continue; // carry on with for loop
        }
      
        // copy chunk of data (from start and end signals 2 and 3) from dataReceived to dataToSend
        // this doesn't bother sending the end character
        // TODO could skip start of data? would have to ignore it in client too
        for (int j = startOfData; j < endOfData; j++) dataToSend[j-startOfData] = dataReceived[i][j];
        // clear the rest of dataToSend array
        for (int j = endOfData - startOfData; j < MAXRECVDATASIZE; j++) dataToSend[j] = '\0';

        // shift back to the start of the dataReceived array any unused data, overwriting data we're sending
        receivedCursor[i] = receivedCursor[i] - endOfData - 1; // -1 because want to overwrite data end char (3)
        for (int j = 0; j < receivedCursor[i]; j++) {
          dataReceived[i][j] = dataReceived[i][j+endOfData+1];
        }
        // clear anything after the cursor
        for (int j = receivedCursor[i]; j < MAXRECVDATASIZE; j++)
          dataReceived[i][j] = '\0';

        if (dataToSend[2] == FLAG_LAYER_FOUND) {
          // this is a message to the server saying a layer was found
          layerFound.push_back((int) dataToSend[3] - 1);
          timeLayerFound.push_back(secondsElapsed());
          cerr << "received layer found: " << layerFound[layerFound.size()-1] << endl;
          for (int k = 0; k < (int) layerFound.size() - 1; k++) {
            if (layerFound[k] == layerFound[layerFound.size()-1]) { // found elsewhere
              // send message saying layer removed
              // if this overwrites new block message then it will overwrite it
              // for both clients
              serverMessage[0] = 2;   // STX
              serverMessage[1] = SERVER_ID; // id
              serverMessage[2] = FLAG_LAYER_REMOVE;
              serverMessage[3] = (char) (layerFound[k] + 1); // don't send null
              serverMessage[4] = 0; // just make sure it ends here
              // remove this layer found thing
              layerFound.pop_back();
              timeLayerFound.pop_back();
              for (int l = k; l < (int) layerFound.size() - 1; l++) {
                layerFound[l] = layerFound[l+1];
                timeLayerFound[l] = timeLayerFound[l+1];
              }
              layerFound.pop_back();
              timeLayerFound.pop_back();
            }
          }
        }else{
          // timeout layer found messages
          if (timeLayerFound.size() > 0) {
            if (secondsElapsed() - timeLayerFound[0] > LAYER_FOUND_EXPIRY_TIME) {
              for (int j = 0; j < (int) timeLayerFound.size() - 1; j++) {
                timeLayerFound[j] = timeLayerFound[j+1];
                layerFound[j] = layerFound[j+1];
              }
              layerFound.pop_back();
              timeLayerFound.pop_back();
              cerr << "layer found message expired" << endl;
            }
          }

          // spit it out
          // keep going through the sockets until all have been done
          for (int j = 0; j < clCount; j++) {

            // if not the person who we received it from (i != j)
            if (i != j) {
              //cout << "sending to socket: " << j << endl;
              //cout << "sending data: " << dataReceived[i] << endl;
              // strlen counts up to and not including the first null character
              if (send(clSocks[j], dataToSend, strlen(dataToSend), 0) == -1) {
                perror("send");
                cerr << "send error" << endl;
              }
            }
          } // end for

        }

      } // end can we send

    } // end if there's received data

  } // end for

  // if there's a message from the server
  if (serverMessage[0] > 0) {
    if (LOG_OUTPUT) cerr << "sending a message to " << clCount << " clients. messNum: " << messNum++ << endl;
    // note this whole function should only be called if all the sockets are writable
    for (int j = 0; j < clCount; j++) {
      // strlen counts up to and not including the first null character
      if (send(clSocks[j], serverMessage, strlen(serverMessage), 0) == -1) {
        perror("send");
        cerr << "send error" << endl;
      }
    }
  }

  // clear server message
  if (serverMessage[0] > 0) {
    if (LOG_OUTPUT) cerr << "message cleared" << endl;
    for (int i = 0; i < MAX_SERVER_MESSAGE_SIZE; i++) serverMessage[i] = '\0';
  }

}

// Do server operations
void doServer()
{
  receiveData();

  // this is bad but only send when we can write to all sockets
  // because otherwise if we send just to one we won't be able to delete data and so it will
  // get sent out again to the same client
  // there is an advantage though for synchronising gravity- server message is only sent out when
  // both clients are ready to hear it (it's not sent to one then the other later)
  bool allWritable = true;
  for (int j = 0; j < clCount; j++) {
    if (!FD_ISSET(clSocks[j], &writeSocks)) allWritable = false;
  }

  if (allWritable) sendData();
}

// Accept a new connection and store in socket array
void serverAccept()
{
  sin_size = sizeof(struct sockaddr_in);

  int check;

  if ((check = accept(listenSock, (struct sockaddr *)&their_addr, (socklen_t*) &sin_size)) == -1) {
    perror("accept");
    cerr << "accept error" << endl;
  }else{
    cerr << "server: got connection from " << inet_ntoa(their_addr.sin_addr) << endl;
    if (clCount < clMax){ // don't allow infinite connections!
      clSocks[clCount] = check;
      //FD_SET(clSocks[clCount], &readSocks); // check this one for readability!
      //if (clSocks[clCount] > maxSock) maxSock = clSocks[clCount]; // adjust maxSock accordingly

      // the connection has been accepted and so give them their id
      char test[4];
      test[0] = 'i';
      test[1] = (char) (clSocks[clCount] % 256);
      if (!assignedMaster) {
        test[2] = FLAG_MASTER;
        assignedMaster = true;
      }else test[2] = FLAG_SLAVE;
      test[3] = '\0';

      //cout << "assigned id: " << (int) test[1] << endl;
      //cout << "test: " << test << endl;
      //cout << "strlen(test): " << strlen(test) << endl;
      if (send(clSocks[clCount], test, strlen(test), 0) == -1) {
        perror("send");
        cerr << "warning: id not sent" << endl;
      }
      
      clCount++; // one more socket!

    }
  }
  
}

// The main function
//   argc - the number of command line arguments
//   argv - the command line arguments
//
// Returns:
//   zero as required by standards
int main(int argc, char** argv)
{
  serverInit();
  
  int numReadable = 0;
  
  while (1) { // keep listening and serving
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_ZERO(&readSocks);
    FD_ZERO(&writeSocks);
    FD_SET(listenSock, &readSocks); // set up the listening sock
    maxSock = listenSock; // this is the only socket to begin with
    // so it must be the maximum one

    for (int i = 0; i < clCount; i++) {
      FD_SET(clSocks[i], &readSocks);
      FD_SET(clSocks[i], &writeSocks);
      // if they've closed a connection, maxSock might decrease
      if (clSocks[i] > maxSock) maxSock = clSocks[i];
    }
  
    //numReadable = select(maxSock + 1, &readSocks, &writeSocks, NULL, &timeout);
    numReadable = select(maxSock + 1, &readSocks, &writeSocks, NULL, NULL);
    //cout << numReadable << endl;
    if (numReadable == -1){
      perror("select");
      cerr << "select error" << endl;
      continue; // ERROR so ignore the rest of loop
    }
    if (numReadable == 0) continue; // keep 'select'ing until we can do something
    
    if (!startedGravity) {
      if (clCount == NUM_PLAYERS) {
        startedGravity = true;
        serverMessage[0] = 2;   // STX
        serverMessage[1] = SERVER_ID; // id
        serverMessage[2] = FLAG_GRAVITY;
        serverMessage[3] = 0; // just make sure it ends here
      }
    }

    // if the listenSock has been set as readable, then accept the incoming connection
    if (FD_ISSET(listenSock, &readSocks)) {
      //cout << "listen sock readable!" << endl;
      serverAccept();
      numReadable--;
      continue; // note this wastes reading free sockets
      // but the reason for it is that nothing can be written unless all sockets are writable
      // and this new socket has not been select()'ed, so calling doServer would lose starting
      // gravity because although it won't be deleted the message will be replaced by new block
    }

    doServer();

    // if it's time and gravity has begun then make new block
    if (clock() > newBlockTimer && startedGravity) newBlock();
    if (clock() > trafficAnalysisTimer) analyseTraffic();
  }

  return 0;
}

