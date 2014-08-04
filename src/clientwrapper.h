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
 * clientwrapper.h
 *
 * This is taken from client.h, which handled network operations directly.
 * Now this is a wrapper for motlab, which is more portable.
 */

#ifndef CLIENTWRAPPER_H
#define CLIENTWRAPPER_H

#include "motlab/outverbose.h"
#include "motlab/client.h"
#include "motlab/net.h"

//#include "pawn.h"
//#include "human.h"
//#include "block.h"
//#include "buffer.h"

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

class ClientWrapper {

  private:
    //int sendPositionCount, sendDelay;
    int id;
    int numbytes, receivedCursor;
    char receivedSoFar[MAXRECVDATASIZE];
    vector <int> humanIds;
    int readyToSend;
    bool master; // master or slave?
    //Buffer sendBuf, moveBuf, manipBuf;
    int sendBlock;
    string oldNetworkData;

    Outverbose out;
    Client client;
    Net net;

  public:
    Client();
    void init();
    char intToChar(int);
    int charToInt(char);
    string dealZeros(float);
    int makeNewHuman(int, vector <Human>&);
    void sendData(string);
    void transmitBufferedData();
    void manipulateBlock(vector <Block>&, int, char, int);
    void moveBlock(vector <Block>&, int, int);
    void doClient(Pawn&, vector <Human>&, vector <Block>&, int&, bool&, bool&, vector <int>&);
    bool getMaster();
    void setHost(const char*);
    void setReadyToSend(int);
    int closeConnection();

};

#endif
