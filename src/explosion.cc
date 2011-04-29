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

#include <GL/glut.h>
#include <iostream>
#include <vector>
#include "explosion.h"

Explosion::Explosion(float initY)
{
  y = initY;
  size = 5.0;
  growSpeed = 0.1;
  alpha = 1.0;
  fadeSpeed = 0.004;
  dead = false;
}

bool Explosion::getDead()
{
  return dead;
}

void Explosion::go()
{
  size += growSpeed;

  alpha -= fadeSpeed;
  if (alpha < 0.05) {
    alpha = 0.0;
    dead = true;
  }
}

void Explosion::draw(vector <int> &texId)
{
  if (!dead) { // explosion may be dead but still called if it's not yet popped from the deque
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, texId[6]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor4f(0.8, 0.8, 0.8, alpha);
    glNormal3f(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-size, y, size);
    glTexCoord2f(1.0, 0.0); glVertex3f(size, y, size);
    glTexCoord2f(1.0, 1.0); glVertex3f(size, y, -size);
    glTexCoord2f(0.0, 1.0); glVertex3f(-size, y, -size);
    glEnd();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  }
}

