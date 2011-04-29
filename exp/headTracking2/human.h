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
 * human.h
 *
 * A human object, storing the details of another user.
 * An avatar is drawn to represent this user.
 *
 * Trevor Dodds, 2005
 */

#ifndef _HUMAN_
#define _HUMAN_

#include <GL/glut.h>
#include <stdlib.h>
#include <iostream>
#include "pawn.h"

//using namespace std;

class Human : public Pawn {
  private:
    //std::vector <Bullet> bullets;
    float targetX, targetY, targetZ, targetAngleX, targetAngleY;
    float lastAmountX, lastAmountY, lastAmountZ;
    int connectionErrorCount;
    bool anim;
    int locked; // which block is locked
    int texFace;

  protected:

  public:
    Human(float, float, float, float, int, int, int);
    void setTargetX(const float);
    void setTargetY(const float);
    void setTargetZ(const float);
    void setTargetAngleX(const float);
    void setTargetAngleY(const float);

    void setLocked(int);
    int getLocked();

    void setTexFace(int);

    void move();

    void draw(std::vector <int>&, int);
};

#endif
