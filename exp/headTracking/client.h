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
 * client.h
 *
 * A client object, designed to deal with the networking side of the CVE.
 * This client implements socket programming which was inspired by a
 * combination of the example code on Beej's Guide to Network Programming
 * http://www.ecst.csuchico.edu/~beej/guide/net/ (last accessed 15/11/2004)
 * and the appendix of Singhal, S. & Zyda, M. (1999) Networked Virtual Environments:
 * Design and Implementation. Reading, MA ; Harlow, England : Addison-Wesley
 *
 * Trevor Dodds, 2005
 */

#ifndef _CLIENT_
#define _CLIENT_

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>
#include "pawn.h"
#include "human.h"
#include "block.h"
#include "buffer.h"

#define PORT 3490 // the port client will be connecting to 
#define MAXRECVDATASIZE 1000 // max number of bytes we can get at once 

// flags for information being transmitted
#define FLAG_NONE ' '
#define FLAG_POSITION 'p'
#define FLAG_LOCK 'l'
#define FLAG_MANIPULATE 'a'
#define FLAG_MOVE 'b'
#define FLAG_BLOCK 'B'

// the following flags are also defined in the server
#define FLAG_MASTER 'M'
#define FLAG_SLAVE 'S'
#define FLAG_NEW_BLOCK 'n'
#define FLAG_GRAVITY 'g'
#define FLAG_GAMEOVER 'G'
#define FLAG_LAYER_FOUND 'L'
#define FLAG_LAYER_REMOVE 'R'

#define SERVER_ID 's'

//using namespace std;

class Client {

  private:
    struct hostent *host;
    struct sockaddr_in serverAddress; // server address information 
    struct timeval timeout; // timeout for using select()
    fd_set readSocks; // sockets to read from
    fd_set writeSocks; // sockets to write to
    int sockfd; // socket file descriptor
    int sendPositionCount, sendDelay, id;
    int numbytes, receivedCursor;
    char receivedSoFar[MAXRECVDATASIZE];
    bool overflow;
    std::vector <int> humanIds;
    int readyToSend;
    bool master; // master or slave?
    Buffer sendBuf, moveBuf, manipBuf;
    int sendBlock;
    std::string oldNetworkData;

  public:
    Client();
    void init();
    char intToChar(int);
    int charToInt(char);
    std::string dealZeros(float);
    int makeNewHuman(int, std::vector <Human>&);
    void sendData(std::string);
    void transmitBufferedData();
    void manipulateBlock(std::vector <Block>&, int, char, int);
    void moveBlock(std::vector <Block>&, int, int);
    void doClient(Pawn&, std::vector <Human>&, std::vector <Block>&, int&, bool&, bool&, std::vector <int>&);
    bool getMaster();
    void setHost(const char*);
    void setReadyToSend(int);
    int closeConnection();

};

#endif
