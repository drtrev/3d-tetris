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
 * pawn.h
 *
 * A Pawn object, specifying the attributes that are common to all world
 * objects including the player, using acceleration and deceleration
 * and inertia. World objects inherit from this.
 *
 * Trevor Dodds, 2005
 */

#ifndef _PAWN_
#define _PAWN_

#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "trig.h"

using namespace std;

#define GAME_SPEED 0.25
//
// Original version tied pawn speed and speed of object rotation to frame rate.
// SECS_PER_FRAME assumes that rate was 122 Hz, to implement speed of movement
// that's independent of framerate
//
#define SECS_PER_FRAME 0.0081967

#define PLAYER_BOUNDARY_MIN_X -200.0
#define PLAYER_BOUNDARY_MIN_Y 0.0
#define PLAYER_BOUNDARY_MIN_Z -200.0
#define PLAYER_BOUNDARY_MAX_X 200.0
#define PLAYER_BOUNDARY_MAX_Y 150.0
#define PLAYER_BOUNDARY_MAX_Z 200.0

class Pawn {
  private:

  protected:
    int id;
    float x, y, z;
    float angleX, angleY, angleZ;
    float turnSpeed;
    float speedX, speedY, speedZ;
    float ground, height, ceil;
    float speed, accel, maxSpeed, strafeRightSpeed;
    float pushX, pushY, pushZ;
    float friction, resistance;
    vector <float> pointHeight;
    float distanceMoved;
      
    void toTarget( float&, float, float );
    void loopAngles();
    void lockPosition(float, float, float, float);

  public:
    Pawn();

    float getX() const;
    float getY() const;
    float getZ() const;
    float getAngleX() const;
    float getAngleY() const;
    float getAngleZ() const;
    float getSpeed() const;
    float getStrafeRightSpeed() const;
    float getMaxSpeed() const;
    float getAccel() const;

    void setX( const float );
    void setY( const float );
    void setZ( const float );
    void setGround( const float );
    void setCeil( const float );
    void setAngleX( const float );
    void setAngleY( const float );
    void setAngleZ( const float );
    void setSpeed( const float );
    void setStrafeRightSpeed( const float );
    void changeGround( const float, const float );

    void accelerate( const float timeSecs );
    void brake( const float timeSecs );
    void strafeRight( const float timeSecs );
    void strafeLeft( const float timeSecs );
    void turnRight();
    void turnLeft();
    void turn(float, float);
    void moveLeft( const float );
    void moveRight( const float );
    void move( const float timeSecs );

    void resetDistanceMoved();
    float getDistanceMoved();
};

#endif
