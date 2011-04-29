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

#ifndef _BLOCK_
#define _BLOCK_

#include <GL/glut.h>
#include <iostream>
#include <vector>
#include "pawn.h"

#define MODE_OBJECT_REFERENCE 0
#define MODE_GLOBAL_REFERENCE 1
#define MODE_VIEWING_REFERENCE 2

// define the boundaries for the game area
#define BOUNDARY_MIN_X -10.0
#define BOUNDARY_MAX_X 10.0
#define BOUNDARY_MIN_Z -10.0
#define BOUNDARY_MAX_Z 10.0

// define the starting height of the blocks
#define BLOCK_START_Y 50.0

// how many blocks can fit into the game?
#define GAMEAREA_WIDTH ((int) (BOUNDARY_MAX_X - BOUNDARY_MIN_X) / 5 + 1)
#define GAMEAREA_DEPTH ((int) (BOUNDARY_MAX_Z - BOUNDARY_MIN_Z) / 5 + 1)
#define GAMEAREA_HEIGHT ((int) BLOCK_START_Y / 5 + 2)

using namespace std;

class Block : public Pawn {
  private:
    int id;
    int type;
    int lockedBy;
    bool remoteMovement;
    vector <vector <vector <int> > > data;
    float matrix[16], confirmedMatrix[16];
    float confirmedX, confirmedY, confirmedZ;
    float oldConfirmedX, oldConfirmedY, oldConfirmedZ;
    float outputMatrix[16];
    float turnedX, turnedY, turnedZ;
    bool turning, moving;
    float targetAngleX, targetAngleY, targetAngleZ;
    float targetX, targetY, targetZ;
    int pivotX, pivotY, pivotZ;
    float oldX, oldY, oldZ;
    float oldMatrix[16];
    int mode;
    float color[4];
    bool interactive; // can block be manipulated by a user?
    bool grounded; // is block grounded? if so don't check collision
    int hitCount; // how many times have we collided?
    vector <vector <int> > newPosition; // store new position of blocks
    vector <vector <int> > lastStored; // store old position/collision trail
    int safelyStoredSize;
    bool arrows; // draw arrows?
    float wallMark[4]; // leave mark on wall on collision
    float wallMarkAlpha;
    bool gotConfirmed;
    bool gameOver;

    bool toTarget(float&, float, float);
    void drawCube();
    void drawArrows();
    void drawArrow();
    void drawCurvedArrow();
    void drawWallMark(vector <int>&);
    bool originalCheckCollision(vector <Block>&, float[]);
    bool checkCollision(int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float[]);
    void clearCollisionTrail(int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], bool);
    void matrixCorrectRoundingError( float* );
    void matrixPrint( const float* );

    void makeShadowObjects();

  protected:
    
  public:
    Block(int, int, float, float, float, bool, int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float[]); // constructor
    
    void setType(int);
    int getType();

    int getData(int, int, int) const;

    void rotateX(float);
    void rotateY(float);
    void rotateZ(float);
    void turn3D(float, float, float);
    void translate(float, float, float);
    void snap();
    void turnGlobalReference( const float timeSecs );
    void turnToTarget();
    void setTargetPosition(float, float, float);
    void changeTargetPosition(float, float, float);
    void getPivotCoords(float&, float&, float&);
    int getPivotX();
    int getPivotY();
    int getPivotZ();
    float getTargetX();
    float getTargetY();
    float getTargetZ();
    void move(vector <Block> &blocks, float[],
      int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], const float);
    bool getMoving();
    void setMode(int);
    void setTargetAngleX(const float);
    void setTargetAngleY(const float);
    void setTargetAngleZ(const float);
    float getTargetAngleX();
    float getTargetAngleY();
    float getTargetAngleZ();
    
    void lock(int);
    void unlock();
    int getLockedBy();

    void setRemoteMovement(bool);
    bool getRemoteMovement();
    
    void output();
    bool getTurning();
    bool getInteractive();
    void setInteractive(bool);
    void setGrounded(bool);
    bool getGrounded();
    int getId();
    bool getGameOver();
    int getNumberOfCubes();

    void draw(vector <int>&, bool);

    void worldCoords(float&, float&, float&);
    float getTurnedX();
    float getTurnedY();
    float getTurnedZ();
    void matrixCopy(const float*, float*);
    float getMatrix(int);
    void getMatrix(float[]);
    void setMatrix(int, float);
    void setConfirmedMatrix(int, float);
    void setConfirmedX(float);
    void setConfirmedY(float);
    void setConfirmedZ(float);
    float getConfirmedMatrix(int);
    float getConfirmedX();
    float getConfirmedY();
    float getConfirmedZ();
    void setGotConfirmed(bool);
    bool getGotConfirmed();
    void toConfirmed(float[], int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH]);
    bool removeLayer(vector <Block>, vector <Block>&, int[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float[], int&, int);
    vector <vector <float> > getLayer(int);
    void hit();
    void unGrounded();

};

#endif
