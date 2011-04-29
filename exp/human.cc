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

#include "human.h"

Human::Human(float setX, float setY, float setZ, float setAngleY, int t, int texFaceInit, int idInit)
{
  // TODO t = type which is not used
  x = setX, y = setY, z = setZ, angleY = setAngleY, id = idInit;
  targetX = 0.0, targetY = 0.0, targetZ = 0.0, targetAngleX = 0.0, targetAngleY = 0.0;
  lastAmountX = 0.0, lastAmountY = 0.0, lastAmountZ = 0.0;
  anim = false;
  connectionErrorCount = 0;
  locked = -1; // nothing locked
  texFace = texFaceInit;
}

void Human::setTargetX(const float n)
{
  targetX = n;
}

void Human::setTargetY(const float n)
{
  targetY = n;
}

void Human::setTargetZ(const float n)
{
  targetZ = n;
}

void Human::setTargetAngleX(const float n)
{
  targetAngleX = n;
}

void Human::setTargetAngleY(const float n)
{
  targetAngleY = n;
}

void Human::setLocked(int n)
{
  locked = n;
}

int Human::getLocked()
{
  return locked;
}

void Human::setTexFace(int n)
{
  texFace = n;
}

void Human::move()
{
  //float step = 20.0;
  // take the average of new amount and last amount so as not to increase too fast
  // or slow down too fast
  if (targetX > x + 200 || targetX < x - 200 || targetY > y + 200 || targetY < y - 200
    || targetZ > z + 200 || targetZ < z - 200) {
    //cout << "hmm... targets set to (xyz): " << targetX << ", " << targetY << ", " << targetZ << endl;
    // are we sure it's jumping this far? wait a while and see if we receive anything nearer current position
    connectionErrorCount--;
    if (connectionErrorCount < 0) {
      x = targetX, y = targetY, z = targetZ; // accept new location
    }
  }else{
    connectionErrorCount = 100; // reset
  
    if (targetX > x + 0.001 || targetX < x - 0.001 || targetY > y + 0.001 || targetY < y - 0.001
      || targetZ > z + 0.001 || targetZ < z - 0.001) {
      anim = true; // animate

      float step = sqrt((targetX - x) * (targetX - x) + (targetZ - z) * (targetZ - z));
      if (step < 0.0001) step = 1.0; // if step is zero, we've not moved in x or z direction
      step *= 5.0;
      // but may have moved in y
      float amountX = ((targetX - x) / step) + (targetX - x) / 50.0; //  + lastAmountX) / 2.0;
      float amountY = ((targetY - y) / step) + (targetY - y) / 50.0; // + lastAmountY) / 2.0;
      float amountZ = ((targetZ - z) / step) + (targetZ - z) / 50.0; // + lastAmountZ) / 2.0;
      amountX = (targetX - x) / 5.0;
      amountY = (targetY - y) / 5.0;
      amountZ = (targetZ - z) / 5.0;

      lastAmountX = amountX, lastAmountY = amountY, lastAmountZ = amountZ;

      x += amountX;
      if (x - amountX < targetX && x > targetX) x = targetX; // don't overshoot the target
      if (x - amountX > targetX && x < targetX) x = targetX;
      y += amountY;
      if (y - amountY < targetY && y > targetY) y = targetY;
      if (y - amountY > targetY && y < targetY) y = targetY;
      //y = targetY; // snap to position
      z += amountZ;
      if (z - amountZ < targetZ && z > targetZ) z = targetZ;
      if (z - amountZ > targetZ && z < targetZ) z = targetZ;
    }else{
      anim = false; // stop anim
      x = targetX, y = targetY, z = targetZ;
    }
  }

  //x = targetX, y = targetY, z = targetZ;
  angleX = targetAngleX;
  angleY = targetAngleY;
  /*if (angleY > 360.0) angleY -= 360.0;
  if (angleY < 0.0) angleY += 360.0;

  float amountAngleY = (targetAngleY - angleY);
  if (amountAngleY < 180.0 && amountAngleY > 0.0) angleY += amountAngleY / 10.0;
  else{
   amountAngleY = targetAngleY - 360 - angleY;
   angleY += amountAngleY / 10.0;
  }*/
}

void Human::draw(vector <int>& texId, int texMain)
{

  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef(angleY, 0.0, 1.0, 0.0);
  glRotatef(-angleX, 1.0, 0.0, 0.0);

  glColor4f(0.8, 0.8, 0.8, 1.0);
  GLfloat matAmbDiff0[] = { 0.8, 0.8, 0.8, 1.0};
  GLfloat matSpecular0[] = { 0.1, 0.1, 0.1, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDiff0);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular0);
  glMaterialf(GL_FRONT, GL_SHININESS, 0);

  glScalef(0.5, 0.5, 0.5);
  //glBindTexture(GL_TEXTURE_2D, texId[texMain]);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);

  // set min and max for texture. Tex coordinates are measured from 0 - 1
  // from the BOTTOM-LEFT of each image!
  float xMin = 0.0, xMax = 92.0/256.0, yMin = 1.0 - 113.0/256.0, yMax = 1.0;
  //cout << xMax << ", " << yMax << endl;
  //xMax = 1.0, yMax = 1.0;
  
  // left
  glNormal3f(-2.0, 0.0, 0.0);
  glTexCoord2f(xMin, yMin); glVertex3f(-5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMin); glVertex3f(-10.0, -10.0, 10.0);
  glTexCoord2f(xMax, yMax); glVertex3f(-10.0, 10.0, 10.0);
  glTexCoord2f(xMin, yMax); glVertex3f(-5.0, 5.0, -7.0);
  
  /*glTexCoord2f(0.0, 0.5); glVertex3f(-10.0, -10.0, -10.0);
  glTexCoord2f(0.3, 0.5); glVertex3f(-10.0, -10.0, 10.0);
  glTexCoord2f(0.3, 1.0); glVertex3f(-10.0, 10.0, 10.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 10.0, -10.0);
*/

  // top
  glNormal3f(0.0, 2.0, 0.0);
  glTexCoord2f(xMin, yMin); glVertex3f(-10.0, 10.0, 10.0);
  glTexCoord2f(xMax, yMin); glVertex3f(10.0, 10.0, 10.0);
  glTexCoord2f(xMax, yMax); glVertex3f(5.0, 5.0, -7.0);
  glTexCoord2f(xMin, yMax); glVertex3f(-5.0, 5.0, -7.0);

  // right
  glNormal3f(2.0, 0.0, 0.0);
  glTexCoord2f(xMin, yMin); glVertex3f(10.0, -10.0, 10.0);
  glTexCoord2f(xMax, yMin); glVertex3f(5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMax); glVertex3f(5.0, 5.0, -7.0);
  glTexCoord2f(xMin, yMax); glVertex3f(10.0, 10.0, 10.0);

  // bottom
  glNormal3f(0.0, -2.0, 0.0);
  glTexCoord2f(xMin, yMin); glVertex3f(-5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMin); glVertex3f(5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMax); glVertex3f(10.0, -10.0, 10.0);
  glTexCoord2f(xMin, yMax); glVertex3f(-10.0, -10.0, 10.0);

  // far side
  glNormal3f(0.0, 0.0, -2.0);
  glTexCoord2f(xMin, yMin); glVertex3f(5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMin); glVertex3f(-5.0, -5.0, -7.0);
  glTexCoord2f(xMax, yMax); glVertex3f(-5.0, 5.0, -7.0);
  glTexCoord2f(xMin, yMax); glVertex3f(5.0, 5.0, -7.0);
  glEnd();

  glEnable(GL_TEXTURE_2D);
  // near side
  xMin = 0.0, xMax = 1.0, yMin = 0.0, yMax = 1.0;
  glBindTexture(GL_TEXTURE_2D, texId[texFace]);
  glNormal3f(0.0, 0.0, 2.0);
  glBegin(GL_QUADS);
  glTexCoord2f(xMin, yMin); glVertex3f(-10.0, -10.0, 10.0);
  glTexCoord2f(xMax, yMin); glVertex3f(10.0, -10.0, 10.0);
  glTexCoord2f(xMax, yMax); glVertex3f(10.0, 10.0, 10.0);
  glTexCoord2f(xMin, yMax); glVertex3f(-10.0, 10.0, 10.0);
  glEnd();

  glPopMatrix();
}

