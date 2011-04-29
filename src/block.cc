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

#include <math.h>
#include "block.h"

// Block constructor
//   initId - the identification number to assign to the block
//   t - the type of the block
//   initX,Y,Z - the initial coordinates for the block
//   initInteractive - boolean value representing whether the block is interactive or not
//   collisionArray - the collision array
//   boundaries - the game area boundaries
Block::Block(int initId, int t, float initX, float initY, float initZ, bool initInteractive, int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float boundaries[])
{
  id = initId; // unique identifier, used in collision detection
  setType(t); // clears and sets data, color and pivotX,Y,Z
  // data is a 5x5x5 3D vector
  x = initX, y = initY, z = initZ;
  oldX = x, oldY = y, oldZ = z;
  confirmedX = x, confirmedY = y, confirmedZ = z;
  oldConfirmedX = confirmedX, oldConfirmedY = confirmedY, oldConfirmedZ = confirmedZ;
  turning = false, moving = false;
  targetAngleX = 0.0, targetAngleY = 0.0, targetAngleZ = 0.0;
  //mode = m;
  arrows = false;
  setMode(MODE_GLOBAL_REFERENCE); // ensures m is valid and sets arrows accordingly
  interactive = initInteractive;
  grounded = false;
  hitCount = 0;
  lockedBy = -1; // unlocked
  remoteMovement = false; // not being moved remotely
  wallMark[0] = 0.0, wallMark[1] = 0.0, wallMark[2] = 0.0, wallMark[3] = 0.0;
  wallMarkAlpha = 0.0;
  gotConfirmed = false;
  gameOver = false;
  speed = 0.15;

  // set up identity matrix
  for (int i = 0, j = 0; i < 16; i++) {
    matrix[i] = 0.0;
    if (i == j) {
      matrix[i] = 1.0;
      j += 5;
    }
  }

  matrixCopy(matrix, oldMatrix);
  matrixCopy(matrix, confirmedMatrix);

  turnedX = 0.0, turnedY = 0.0, turnedZ = 0.0;

  lastStored.clear();
  float wx = 0, wy = 0, wz = 0;
  int worldX = 0, worldY = 0, worldZ = 0;
  vector <int> pos;

  // go through block array
  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (data[x][y][z] == 1) { // if there's a block here
          // get world coords of current block
          wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
          worldCoords(wx, wy, wz);
          worldX = (int) roundf((wx - boundaries[0]) / 5.0), worldY = (int) roundf(wy / 5.0), worldZ = (int) roundf((wz - boundaries[2]) / 5.0);

          // check we're in range (to stop any annoying segfaults)
          if (worldX < 0 || worldY < 0 || worldZ < 0 ||
              //worldX >= (int) collisionArray.size() || worldY >= (int) collisionArray[0].size() ||
              //worldZ >= (int) collisionArray[0][0].size()) {
              worldX >= GAMEAREA_WIDTH || worldY >= GAMEAREA_HEIGHT ||
              worldZ >= GAMEAREA_DEPTH) {
            cerr << "Block::Block - attempt to check a world coordinate outside of collision array bounds" << endl;
            cerr << "worldX: " << worldX << ", worldY: " << worldY << ", worldZ: " << worldZ << endl;
          }else{
            if (collisionArray[worldX][worldY][worldZ] > 0) {
              //cout << "Block constructor detected GAME OVER" << endl;
              gameOver = true;
            }
            collisionArray[worldX][worldY][worldZ] = id;
            pos.clear();
            pos.push_back(worldX);
            pos.push_back(worldY);
            pos.push_back(worldZ);
            lastStored.push_back(pos);
          }
        }
      }
    }
  }

  // we've put the block in the collision array, so store the size of
  // lastStored
  safelyStoredSize = lastStored.size();
}

// Sets the type of the block and creates its shape and colour based on this
//   t - the type of the block
void Block::setType(int t)
{
  type = t;

  vector <int> temp; // temporary store to initialise 2D vector
  vector <vector <int> > temp2; // temporary store to initialise 3D vector

  data.clear();

  // make 5x5x5 3D vector
  temp.push_back(0);
  temp.push_back(0);
  temp.push_back(0);
  temp.push_back(0);
  temp.push_back(0);
  temp2.push_back(temp);
  temp2.push_back(temp);
  temp2.push_back(temp);
  temp2.push_back(temp);
  temp2.push_back(temp);
  data.push_back(temp2);
  data.push_back(temp2);
  data.push_back(temp2);
  data.push_back(temp2);
  data.push_back(temp2);

  switch (t) {
    case 1: // corner block
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[0][1][0] = 1;
      data[0][0][1] = 1;
      color[0] = 0.8, color[1] = 0.5, color[2] = 0.5, color[3] = 1.0;
      pivotX = 0, pivotY = 0, pivotZ = 0;
      break;
    case 2: // L shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[2][0][0] = 1;
      data[0][0][1] = 1;
      color[0] = 0.5, color[1] = 0.8, color[2] = 0.5, color[3] = 1.0;
      pivotX = 5, pivotY = 0, pivotZ = 0;
      break;
    case 3: // kind of S shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[0][1][0] = 1;
      data[0][1][1] = 1;
      color[0] = 0.2, color[1] = 0.7, color[2] = 1.0, color[3] = 1.0;
      pivotX = 0, pivotY = 0, pivotZ = 0;
      break;
    case 4: // other kind of S shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[0][1][0] = 1;
      data[1][0][1] = 1;
      color[0] = 1.0, color[1] = 1.0, color[2] = 0.4, color[3] = 1.0;
      pivotX = 0, pivotY = 0, pivotZ = 0;
      break;
    case 5: // Z shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[1][1][0] = 1;
      data[2][1][0] = 1;
      color[0] = 0.8, color[1] = 0.5, color[2] = 0.8, color[3] = 1.0;
      pivotX = 5, pivotY = 0, pivotZ = 0;
      break;
    case 6: // T shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[2][0][0] = 1;
      data[1][1][0] = 1;
      color[0] = 0.5, color[1] = 0.8, color[2] = 0.8, color[3] = 1.0;
      pivotX = 5, pivotY = 0, pivotZ = 0;
      break;
    case 7: // Small L shape
      data[0][0][0] = 1;
      data[1][0][0] = 1;
      data[0][1][0] = 1;
      color[0] = 0.7, color[1] = 0.7, color[2] = 1.0, color[3] = 1.0;
      pivotX = 0, pivotY = 0, pivotZ = 0;
      break;
    default: // type 0, just one block
      data[0][0][0] = 1; // set to pivot point
      color[0] = 0.8, color[1] = 0.8, color[2] = 0.8, color[3] = 1.0;
      pivotX = 0, pivotY = 0, pivotZ = 0;
      break;
  }

}

int Block::getType()
{
  return type;
}

// Gets the block data at a specified array location
//   x,y,z - the units of data to access
int Block::getData(int x, int y, int z) const
{
  if (x > -1 && x < 5 && y > -1 && y < 5 && z > -1 && 5) {
    return data[x][y][z];
  }else{
    return -1; // out of range
  }
}

// Rotate the block about the X axis by changing its matrix
//   angle - the angle to rotate in degrees
void Block::rotateX(float angle)
{
  glPushMatrix();
  glLoadIdentity();
  glRotatef(angle, 1.0, 0.0, 0.0);
  glMultMatrixf(matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();

  turnedX += angle;
}

// Rotate the block about the Y axis by changing its matrix
//   angle - the angle to rotate in degrees
void Block::rotateY(float angle)
{
  glPushMatrix();
  glLoadIdentity();
  glRotatef(angle, 0.0, 1.0, 0.0);
  glMultMatrixf(matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();

  turnedY += angle;
}

// Rotate the block about the Z axis by changing its matrix
//   angle - the angle to rotate in degrees
void Block::rotateZ(float angle)
{
  glPushMatrix();
  glLoadIdentity();
  glRotatef(angle, 0.0, 0.0, 1.0);
  glMultMatrixf(matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();

  turnedZ += angle;
}

// Move the block by changing its matrix
//   x,y,z - the direction in which to move the block
void Block::translate(float x, float y, float z)
{
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(x, y, z);
  glMultMatrixf(matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();
}

// Snap the block to position
void Block::snap()
{
  while (turnedX < 0.0) turnedX += 360.0;
  while (turnedY < 0.0) turnedY += 360.0;
  while (turnedZ < 0.0) turnedZ += 360.0;
  while (turnedX > 360.0) turnedX -= 360.0;
  while (turnedY > 360.0) turnedY -= 360.0;
  while (turnedZ > 360.0) turnedZ -= 360.0;

  if (turnedX < 45) rotateX(-turnedX);
  else if (turnedX < 135) rotateX(90-turnedX);
  else if (turnedX < 225) rotateX(180-turnedX);
  else if (turnedX < 315) rotateX(270-turnedX);
  else rotateX(360-turnedX);

  if (turnedY < 45) rotateY(-turnedY);
  else if (turnedY < 135) rotateY(90-turnedY);
  else if (turnedY < 225) rotateY(180-turnedY);
  else if (turnedY < 315) rotateY(270-turnedY);
  else rotateY(360-turnedY);
  /*if (turnedX < 45) rotateX(-turnedX);
  else if (turnedX < 90) rotateX(turnedX-45);
  else if (turnedX < 135) rotateX(-turnedX+90);
  else if (turnedX < 180) rotateX(turnedX-135);
  else if (turnedX < 225) rotateX(-turnedX+180);
  else if (turnedX < 270) rotateX(turnedX-225);
  else if (turnedX < 315) rotateX(-turnedX+270);
  else rotateX(turnedX-315);*/
  
  if (angleX < -135) angleX = -180.0;
  else if (angleX < -45) angleX = -90.0;
  else if (angleX < 45) angleX = 0.0;
  else if (angleX < 135) angleX = 90.0;
  else angleX = 180;

  if (angleY < 45) angleY = 0;
  else if (angleY < 135) angleY = 90;
  else if (angleY < 225) angleY = 180;
  else if (angleY < 315) angleY = 270;
  else angleY = 0;

  if (angleZ < 45) angleZ = 0;
  else if (angleZ < 135) angleZ = 90;
  else if (angleZ < 225) angleZ = 180;
  else if (angleZ < 315) angleZ = 270;
  else angleZ = 0;

}

// Change a variable towards a target
//   var - the variable to change
//   amount - the amount to increase or decrease var by
//   target - the target value - if it overshoots then it is set back to this
//
// NB: pawn.cc has parameters of similar function in order (var, target, amount)
//     and also doesn't return a boolean (RAR)
//
bool Block::toTarget(float &var, float amount, float target)
{
  if (var < target) {
    var += amount;
    if (var > target) var = target;
  }
  else if (var > target) {
    var -= amount;
    if (var < target) var = target;
  }
  if (var == target) return true;
  else return false;
}

// Turn the block in a global frame of reference
void Block::turnGlobalReference( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  //
  // Old version hard coded an angle increment of 1.0
  //
  //float angle = (float) ((int) (1.0 * timeSecs / SECS_PER_FRAME));
  float angle = 1.0 * timeSecs / SECS_PER_FRAME;

  if (targetAngleX > 0) {
    toTarget(targetAngleX, angle, 0.0);
    rotateX(angle);
    //cout << " " << targetAngleX;
  }else if (targetAngleX < 0) {
    toTarget(targetAngleX, angle, 0.0);
    rotateX(-angle);
    //cout << " " << targetAngleX;
  }else if (targetAngleY > 0) {
    toTarget(targetAngleY, angle, 0.0);
    rotateY(angle);
  }else if (targetAngleY < 0) {
    toTarget(targetAngleY, angle, 0.0);
    rotateY(-angle);
  }else if (targetAngleZ > 0) {
    toTarget(targetAngleZ, angle, 0.0);
    rotateZ(angle);
  }else if (targetAngleZ < 0) {
    toTarget(targetAngleZ, angle, 0.0);
    rotateZ(-angle);
  }else{
    turning = false;
    matrixCorrectRoundingError( matrix ); // Hack (RAR)
    matrixCopy(matrix, oldMatrix);
    //matrixPrint( matrix );
  }
}

// Turn the block to its target angle
void Block::turnToTarget()
{
  cout << "ho!" << endl;
  bool target1 = toTarget(angleX, 0.2, targetAngleX);
  bool target2 = toTarget(angleY, 0.2, targetAngleY);
  bool target3 = toTarget(angleZ, 0.2, targetAngleZ);
  if (target1 && target2 && target3) {
    turning = false;
    matrixCopy(matrix, oldMatrix);
    cout << "before loop: x,y,z: " << angleX << ", " << angleY << ", " << angleZ << endl;
    loopAngles();
    targetAngleX = angleX, targetAngleY = angleY, targetAngleZ = angleZ;
    cout << "after loop: x,y,z: " << angleX << ", " << angleY << ", " << angleZ << endl;
  }
}

// Move the block
//   blocks - the vector storing all the game blocks
//   boundaries - the game boundaries
//   collisionArray - the array used for collision detection purposes
void Block::move(vector <Block> &blocks, float boundaries[],
  int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH],
  const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  //
  // Moves blocks at speed that's independent of framerate
  //
  float dist = speed * timeSecs / SECS_PER_FRAME;

  if (turning) {
    switch (mode) {
      case MODE_OBJECT_REFERENCE:
        //
        // Not now used, so framerate-independent movement speed not
        // implemented (RAR)
        //
        turnToTarget();
        break;
      case MODE_GLOBAL_REFERENCE:
        turnGlobalReference( timeSecs );
        break;
      case MODE_VIEWING_REFERENCE:
        // not yet implemented
        break;
    }

    // no longer turning, clear collision trail and 'true': store newPosition
    if (!turning) clearCollisionTrail(collisionArray, true);
  }

  if (moving) {
    //
    // did move at 0.1, now moves at speed
    // Changed 'speed' to 'dist' so movement is at speed independent of
    // framerate (RAR)
    //
    if (x < targetX) {
      //translate(1, 0, 0);
      toTarget(x, dist, targetX);
    }else if (x > targetX) {
      //translate(-1, 0, 0);
      toTarget(x, dist, targetX);
    }else if (y < targetY) {
      //translate(0, 1, 0);
      toTarget(y, dist, targetY);
    }else if (y > targetY) {
      //translate(0, -1, 0);
      toTarget(y, dist, targetY);
    }else if (z < targetZ) {
      //translate(0, 0, 1);
      toTarget(z, dist, targetZ);
    }else if (z > targetZ) {
      //translate(0, 0, -1);
      toTarget(z, dist, targetZ);
    }else{
      x = roundf(x), y = roundf(y), z = roundf(z);
      oldX = x, oldY = y, oldZ = z;
      moving = false;
      clearCollisionTrail(collisionArray, true);
    }
  }

  if (turning || moving) {
    //if (checkCollision(blocks, boundaries)) hit();
    if (checkCollision(collisionArray, boundaries)) hit();
  }
    
  // collision leaves a mark on the wall which fades away
  if (wallMarkAlpha > 0.0) wallMarkAlpha -= 0.01;
}

// Get block's moving status
// 
// Returns:
//   a boolean identifying whether the block is moving (true) or stationary (false)
bool Block::getMoving()
{
  return moving;
}

// Set the mode of the block
//   m - the mode of the block, set to an integer that corresponds to one of the mode constants
void Block::setMode(int m)
{
  if (m == MODE_VIEWING_REFERENCE || m == MODE_GLOBAL_REFERENCE || m == MODE_OBJECT_REFERENCE) {
    mode = m;

    if (mode == MODE_GLOBAL_REFERENCE) {
      //arrows = false;
      targetAngleX = 0, targetAngleY = 0, targetAngleZ = 0;
      angleX = 0, angleY = 0, angleZ = 0;
      // TODO this is set up in constructor also - perhaps it should only be done here??
      // set up identity matrix
      for (int i = 0, j = 0; i < 16; i++) {
        matrix[i] = 0.0;
        if (i == j) {
          matrix[i] = 1.0;
          j += 5;
        }
      }
    }
  }
}

// Set the target position of the block
//   nx,y,z - the coordinates of the target position
void Block::setTargetPosition(float nx, float ny, float nz)
{
  oldX = x, oldY = y, oldZ = z;
  targetX = nx, targetY = ny, targetZ = nz;
  //targetX = targetX / 5 * 5;
  //targetY = targetY / 5 * 5;
  //targetZ = targetZ / 5 * 5;
  moving = true;
}

// Change the target position of the block
// (Similar to setTargetPosition but doesn't set old position or start movement)
//   nx,y,z - the coordinates of the target position
void Block::changeTargetPosition(float nx, float ny, float nz)
{
  targetX = nx, targetY = ny, targetZ = nz;
}

// Get the coordinates of the pivot cube
//   wx,y,z - the coordinates of the pivot cube are assigned to these variables
void Block::getPivotCoords(float &wx, float &wy, float &wz)
{
  wx = pivotX, wy = pivotY, wz = pivotZ;
  worldCoords(wx, wy, wz);
}

int Block::getPivotX()
{
  return pivotX;
}

int Block::getPivotY()
{
  return pivotY;
}

int Block::getPivotZ()
{
  return pivotZ;
}

// Get the x target coordinate
float Block::getTargetX()
{
  return targetX;
}

// Get the y target coordinate
float Block::getTargetY()
{
  return targetY;
}

// Get the z target coordinate
float Block::getTargetZ()
{
  return targetZ;
}

// Set the target x angle
//   a - the target angle
void Block::setTargetAngleX(const float a)
//
// Sets target orientation for block to turn to. After inspecting
// mouseActiveMove() in cve.cc, and printing some diagnostics,
// 'a' is always 90 or -90
//
{
  targetAngleX = a;
  //cout << endl << "x " << targetAngleX << endl;
  turning = true;
}

// Set the target y angle
//   a - the target angle
void Block::setTargetAngleY(const float a)
{
  targetAngleY = a;
  //cout << "y " << targetAngleY << endl;
  turning = true;
}

// Set the target z angle
//   a - the target angle
void Block::setTargetAngleZ(const float a)
{
  targetAngleZ = a;
  //cout << "z " << targetAngleZ << endl;
  turning = true;
}

// Get the target x angle
float Block::getTargetAngleX()
{
  return targetAngleX;
}

// Get the target y angle
float Block::getTargetAngleY()
{
  return targetAngleY;
}

// Get the target z angle
float Block::getTargetAngleZ()
{
  return targetAngleZ;
}

// Change the angles of orientation
//   turnX - the amount to turn in the x axis
//   turnY - the amount to turn in the y axis
//   turnZ - the amount to turn in the z axis
void Block::turn3D(float turnX, float turnY, float turnZ)
{
  // don't lock angles on this one
  angleX += turnX;
  angleY += turnY;
  //angleZ += round * (90.0 - angleY) / 90.0;
  angleZ += turnZ;
  loopAngles();
}

// Set block as locked by a particular user
//   i - the user id of the user who is locking this block
void Block::lock(int i)
{
  lockedBy = i;
}

// Unlock the block
void Block::unlock()
{
  lockedBy = -1;
}

// Return the user id who has locked this block
int Block::getLockedBy()
{
  return lockedBy;
}

// Set remote movement property
//   b - the value to assign
void Block::setRemoteMovement(bool b)
{
  remoteMovement = b;
}

// Return the remote movement property
bool Block::getRemoteMovement()
{
  return remoteMovement;
}

// Output data for testing purposes
void Block::output()
{
  cout << "Turned X: " << turnedX << ", turnedY: " << turnedY << ", turnedZ: " << turnedZ << endl;
  cout << "Angle X: " << angleX << ", angleY: " << angleY << ", angleZ: " << angleZ << endl;
  cout << "Turning: " << turning << endl;
  for (int i = 0; i < 16; i++) {
    if (i%4 == 0) cout << endl;
    cout << outputMatrix[i] << ", ";
  }
  cout << endl;
  float wx = 0, wy = 0, wz = 0;
  worldCoords(wx, wy, wz);
  cout << "World coords of (0, 0, 0): " << roundf(wx) << ", " << roundf(wy) << ", " << roundf(wz) << endl;
  wx = 1, wy = 0, wz = 0;
  worldCoords(wx, wy, wz);
  cout << "World coords of (1, 0, 0): " << roundf(wx) << ", " << roundf(wy) << ", " << roundf(wz) << endl;
}

// Get the turning property
bool Block::getTurning()
{
  return turning;
}

// Get the interactive property
bool Block::getInteractive()
{
  return interactive;
}

// Set the interactive property
//   b - the value to assign
void Block::setInteractive(bool b)
{
  interactive = b;
}

void Block::setGrounded(bool g)
{
  grounded = g;
}
  
// Get the grounded property
bool Block::getGrounded()
{
  return grounded;
}

// Get the block's id
int Block::getId()
{
  return id;
}

// Get the game over property
bool Block::getGameOver()
{
  return gameOver;
}

// Get the number of cubes that make up the block
int Block::getNumberOfCubes()
{
  // count the number of cubes
  int count = 0;

  // go through block array
  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (data[x][y][z] == 1) { // if there's a block here
          count++;
        }
      }
    }
  }

  return count;
}

// Draw a curved arrow representing direction of rotation
void Block::drawCurvedArrow()
{
  glNormal3f(0.0, 0.0, 1.0);

  glBegin(GL_TRIANGLES);
  glVertex3f(-3.0, -2.0, 0.0);
  glVertex3f(-2.0, -2.0, 0.0);
  glVertex3f(-3.0, -1.0, 0.0);
  glEnd();
  glBegin(GL_QUAD_STRIP);
  glVertex3f(-2.6, -1.4, 0.0);
  glVertex3f(-2.4, -1.6, 0.0);
  glVertex3f(-1.0, 0.0, 0.0);
  glVertex3f(-1.0, -0.2, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, -0.2, 0.0);
  glVertex3f(2.2, -0.8, 0.0);
  glVertex3f(2.2, -1.0, 0.0);
  glEnd();
}

// draw a flat arrow representing direction of translation
void Block::drawArrow()
{
  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);

  glNormal3f(0.3, 0.0, 0.9);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(2.0, 0.0, 0.5);
  glVertex3f(2.0, 1.0, 0.5);
  glVertex3f(1.0, 1.0, 0.0);

  glNormal3f(0.7, 0.0, 0.7);
  glVertex3f(2.0, 0.0, 0.5);
  glVertex3f(3.0, 0.0, 1.5);
  glVertex3f(3.0, 1.0, 1.5);
  glVertex3f(2.0, 1.0, 0.5);
  glEnd();

  glBegin(GL_TRIANGLES);
  glNormal3f(0.9, 0.0, 0.3);
  glVertex3f(3.0, -0.5, 1.5);
  glVertex3f(3.5, 0.5, 2.5);
  glVertex3f(3.0, 1.5, 1.5);
  glEnd();

  /*glNormal3f(0.0, 0.0, 1.0);
  glBegin(GL_POLYGON);
  glVertex3f(-1.0, 0.0, 0.0);
  glVertex3f(0.0, -1.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glEnd();

  glBegin(GL_POLYGON);
  glVertex3f(0.5, 0.0, 0.0);
  glVertex3f(0.5, 1.0, 0.0);
  glVertex3f(-0.5, 1.0, 0.0);
  glVertex3f(-0.5, 0.0, 0.0);
  glEnd();*/

  // other side
  /*glNormal3f(0.0, 0.0, -1.0);
  glBegin(GL_POLYGON);
  glVertex3f(-1.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, -1.0, 0.0);
  glEnd();

  glBegin(GL_POLYGON);
  glVertex3f(0.5, 0.0, 0.0);
  glVertex3f(-0.5, 0.0, 0.0);
  glVertex3f(-0.5, 1.0, 0.0);
  glVertex3f(0.5, 1.0, 0.0);
  glEnd();*/
}

// Draw arrows representing directions of manipulation
void Block::drawArrows()
{
  glPushMatrix();
  glTranslatef(pivotX, pivotY, pivotZ);
  glScalef(-2.0, 2.0, 2.0);
  glPushMatrix();
  //glTranslatef(0, -2, 0);
  glColor4f(0.8, 0.5, 0.5, 1.0);
  drawArrow(); // red right
  //glTranslatef(0, 4, 0);
  //glRotatef(180, 0.0, 0.0, 1.0);
  glRotatef(180, 0.0, 1.0, 0.0);
  //glScalef(-1.0, 1.0, 1.0);
  //glCullFace(GL_FRONT);
  drawArrow(); // back red
  //glCullFace(GL_BACK);
  glPopMatrix();
  glPushMatrix();
  //glTranslatef(2, 0, 0);
  glRotatef(90, 0.0, 0.0, 1.0);
  glColor4f(0.8, 0.8, 0.5, 1.0);
  drawArrow(); // yellow up
  //glTranslatef(0, 4, 0);
  glRotatef(180, 0.0, 1.0, 0.0);
  //glRotatef(180, 0.0, 0.0, 1.0);
  drawArrow(); // yellow back
  glPopMatrix();
  glTranslatef(0, 2, 0);
  glColor4f(0.5, 0.5, 0.8, 1.0);
  glScalef(-1.0, 1.0, -1.0);
  drawCurvedArrow(); // blue clockwise
  glTranslatef(0, -4, 0);
  glScalef(1.0, -1.0, 1.0);
  glRotatef(180, 0.0, 1.0, 0.0);
  glCullFace(GL_FRONT);
  drawCurvedArrow();
  glCullFace(GL_BACK);
  glTranslatef(-pivotX, -pivotY, -pivotZ);
  glPopMatrix();
}

// Draw individual block cube
void Block::drawCube()
{
  float texMin = 0.0, texMax = 0.2;
  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(texMin, texMin); glVertex3f(-2.5, -2.5, 2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(2.5, -2.5, 2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(2.5, 2.5, 2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(-2.5, 2.5, 2.5);

  glNormal3f(1.0, 0.0, 0.0);
  glTexCoord2f(texMin, texMin); glVertex3f(2.5, -2.5, 2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(2.5, 2.5, -2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(2.5, 2.5, 2.5);

  glNormal3f(0.0, -1.0, 0.0);
  glTexCoord2f(texMin, texMin); glVertex3f(-2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(2.5, -2.5, 2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(-2.5, -2.5, 2.5);

  glNormal3f(-1.0, 0.0, 0.0);
  glTexCoord2f(texMin, texMin); glVertex3f(-2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(-2.5, -2.5, 2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(-2.5, 2.5, 2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(-2.5, 2.5, -2.5);

  glNormal3f(0.0, 0.0, -1.0);
  glTexCoord2f(texMin, texMin); glVertex3f(2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(-2.5, -2.5, -2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(-2.5, 2.5, -2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(2.5, 2.5, -2.5);

  glNormal3f(0.0, 1.0, 0.0);
  glTexCoord2f(texMin, texMin); glVertex3f(-2.5, 2.5, 2.5);
  glTexCoord2f(texMax, texMin); glVertex3f(2.5, 2.5, 2.5);
  glTexCoord2f(texMax, texMax); glVertex3f(2.5, 2.5, -2.5);
  glTexCoord2f(texMin, texMax); glVertex3f(-2.5, 2.5, -2.5);
  glEnd();
}

// Draw wall mark left by collision
//   texId - the texture id's
void Block::drawWallMark(vector <int> &texId)
{
  glPushMatrix();

  glBindTexture(GL_TEXTURE_2D, texId[6]);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDisable(GL_CULL_FACE);
  glColor4f(0.8, 0.8, 0.8, wallMarkAlpha);

  glNormal3f(1.0, 0.0, 0.0);
  glTranslatef(wallMark[0], wallMark[1], wallMark[2]);

  switch((int) wallMark[3]) {
    case 1:
      glRotatef(90, 0.0, 1.0, 0.0);
      break;
    case 2:
      glRotatef(180, 0.0, 1.0, 0.0);
      break;
    case 3:
      glRotatef(270, 0.0, 1.0, 0.0);
      break;
  }
      
  float size = 5.0;
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size, 2.5);
  glTexCoord2f(1.0, 0.0); glVertex3f(size, -size, 2.5);
  glTexCoord2f(1.0, 1.0); glVertex3f(size, size, 2.5);
  glTexCoord2f(0.0, 1.0); glVertex3f(-size, size, 2.5);
  glEnd();

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);

  glPopMatrix();
}

// Draw the block
//   texId - the texture id's
//   selected - defines whether the block is selected (true) or not (false)
void Block::draw(vector <int>& texId, bool selected)
{
  glPushMatrix();
  //glLoadIdentity();
  glTranslatef(x, y, z); // move to position
  glPushMatrix(); // retain current matrix for arrows

  glTranslatef(pivotX, pivotY, pivotZ); // move to pivot point

  if (mode == MODE_GLOBAL_REFERENCE) glMultMatrixf(matrix);

  //glPushMatrix();
  //glLoadIdentity();
  //glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  //glPopMatrix();

  //rotateX(angleX);
  //rotateY(angleY);
  //rotateZ(angleZ);
  if (mode == MODE_OBJECT_REFERENCE) {
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, 1.0, 0.0);
    glRotatef(angleZ, 0.0, 0.0, 1.0);
  }
  //glMultMatrixf(matrix);

  glTranslatef(-pivotX, -pivotY, -pivotZ); // move back (from pivot)

  glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix);
  
  glColor4fv(color);
  // set material properties
  GLfloat matAmbDiff0[] = { 0.8, 0.8, 0.8, 1.0};
  GLfloat matSpecular0[] = { 0.2, 0.2, 0.2, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDiff0);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular0);
  glMaterialf(GL_FRONT, GL_SHININESS, 10);

  if (interactive) glBindTexture(GL_TEXTURE_2D, texId[4]);
  else glBindTexture(GL_TEXTURE_2D, texId[6]);

  /*glBegin(GL_QUADS);
  glNormal3f(0.0, 1.0, 0.0);
  glVertex3f(-20.0, 1.0, 1.0);
  glVertex3f(20.0, 1.0, 1.0);
  glVertex3f(20.0, 1.0, -1.0);
  glVertex3f(-20.0, 1.0, -1.0);

  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(-20.0, -1.0, 1.0);
  glVertex3f(20.0, -1.0, 1.0);
  glVertex3f(20.0, 1.0, 1.0);
  glVertex3f(-20.0, 1.0, 1.0);

  glNormal3f(0.0, 0.0, -1.0);
  glVertex3f(-20.0, 1.0, -1.0);
  glVertex3f(20.0, 1.0, -1.0);
  glVertex3f(20.0, -1.0, -1.0);
  glVertex3f(-20.0, -1.0, -1.0);
  
  glNormal3f(0.0, -1.0, 0.0);
  glVertex3f(-20.0, -1.0, 1.0);
  glVertex3f(-20.0, -1.0, -1.0);
  glVertex3f(20.0, -1.0, -1.0);
  glVertex3f(20.0, -1.0, 1.0);
  glEnd();*/

  for (int k = 0; k < 5; k++) {
    for (int j = 0; j < 5; j++) {
      for (int i = 0; i < 5; i++) {

        if (data[i][j][k] == 1) {
          glPushMatrix();
          glTranslatef(i * 5.0, j * 5.0, k * 5.0);
          //glutSolidCube(5.0);
          if (i * 5 == pivotX && j * 5 == pivotY && k * 5 == pivotZ && selected && !arrows) {
            // draw triangle in pivot block
            /*glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            glColor4f(0.2, 0.2, 0.2, 1.0);
            glBegin(GL_TRIANGLES);
            glNormal3f(0.0, 0.0, 1.0);
            glVertex3f(-0.5, -0.5, 0.0);
            glVertex3f(0.5, -0.5, 0.0);
            glVertex3f(0.0, 0.5, 0.0);
            glEnd();
            
            glColor4fv(color);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);*/
            // highlight pivot block
            glColor4f(color[0]*1.2, color[1]*1.2, color[2]*1.2, 1.0);
            //glDisable(GL_TEXTURE_2D);
          }else{
            glColor4fv(color);
            //glEnable(GL_TEXTURE_2D);
          }
          drawCube();
          glPopMatrix();
        }

      }
    }
  }

  //glEnable(GL_TEXTURE_2D);

  // draw arrows
  glPopMatrix(); // end object matrix, still in initial matrix

  if (selected && arrows) {
    glDisable(GL_DEPTH_TEST); // so block can go through arrows and they'll still be seen
    glDisable(GL_TEXTURE_2D);

    drawArrows();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
  }

  glPopMatrix();

  if (wallMarkAlpha > 0.0) {
    drawWallMark(texId);
  }
}

// Get the world coordinates of the block
//   wx,y,z - world coordinates are stored in these parameters
void Block::worldCoords(float &wx, float &wy, float &wz)
{
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(x + pivotX, y + pivotY, z + pivotZ); // change pivot point
  glMultMatrixf(matrix);
  glTranslatef(wx - pivotX, wy - pivotY, wz - pivotZ);
  glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix);
  glPopMatrix();
  /*float coordVector[3];

  for (int i = 0; i < 3; i += 4) {
    coordVector[i] = matrix[i] * wx + matrix[i+1] * wy + matrix[i+2] * wz + matrix[i+3];
  }*/
  //for (int i = 0; i < 16; i++) {
    //if (i%4 == 0) cout << endl;
    //cout << outputMatrix[i] << ", ";
  //}

  // TODO outputMatrix doesn't have to be global
  wx = outputMatrix[12], wy = outputMatrix[13], wz = outputMatrix[14];
}

float Block::getTurnedX()
{
  return turnedX;
}

float Block::getTurnedY()
{
  return turnedY;
}

float Block::getTurnedZ()
{
  return turnedZ;
}

// Copy one matrix to another
//   m1 - the matrix to copy
//   m2 - the matrix to fill
void Block::matrixCopy(const float m1[16], float m2[16])
{
  for (int i = 0; i < 16; i++) m2[i] = m1[i];
}

void Block::matrixCorrectRoundingError(float m1[])
//
// Hack to correct for rounding error caused by incremental rotations.
// If rotational element (3x3) of matrix is < small no. then set to zero
//
{
  int l1, l2, element;
  const float ROUNDING = 0.1;

  for( l1=0; l1<3; l1++ ) { // Columns

    for( l2=0; l2<3; l2++ ) { // Rotational elements in column
      element = l1 * 4 + l2;

      if( fabsf( m1[element] ) < ROUNDING ) {
        m1[element] = 0.0;
      }
      else if( fabsf( fabsf( m1[element] ) - 1.0 ) < ROUNDING ) {
        m1[element] = m1[element] < 0.0 ? -1.0 : 1.0;
      }
      else {
        cout << "Block::matrixCorrectRoundingError " << m1[element] << endl;
      }
    }
  }
}

void Block::matrixPrint( const float m1[] )
//
// Print 4x4 matrix
//
{
  int l1;
  cout << endl;
                                                                                
  for( l1=0; l1<16; l1++ ) {
    cout << matrix[l1];
                                                                                
    if( (l1 + 1) % 4 ) {
      cout << '\t';
    }
    else {
      cout << endl;
    }
  }
}

// Return an element of the matrix
//   i - the element to return
float Block::getMatrix(int i)
{
  return matrix[i];
}

void Block::getMatrix(float m[])
{
  for (int i = 0; i < 16; i++) {
    m[i] = matrix[i];
  }
}

// Set an element of the matrix
//   i - the element to set
//   v - the value for this element
void Block::setMatrix(int i, float v)
{
  if (i < 0 || i > 15) {
    cerr << "Block::setMatrix: out of range: " << i << endl;
  }else
    matrix[i] = v;
}

// Set an element of the confirmed matrix
//   i - the element to set
//   v - the value for this element
void Block::setConfirmedMatrix(int i, float v)
{
  if (i < 0 || i > 15) {
    cerr << "Block::setConfirmedMatrix: out of range: " << i << endl;
  }else
    confirmedMatrix[i] = v;
}

// Set the confirmed x property
//   n - the value to assign
void Block::setConfirmedX(float n)
{
  confirmedX = n;
}

// Set the confirmed y property
//   n - the value to assign
void Block::setConfirmedY(float n)
{
  confirmedY = n;
}

// Set the confirmed z property
//   n - the value to assign
void Block::setConfirmedZ(float n)
{
  confirmedZ = n;
}

// Return an element of the confirmed matrix
//   i - the element number to return
float Block::getConfirmedMatrix(int i)
{
  if (i < 0 || i > 15) {
    cerr << "Block::getConfirmedMatrix: out of range: " << i << endl;
    return 0.0;
  }else
    return confirmedMatrix[i];
}

// Get confirmed x property
float Block::getConfirmedX()
{
  return confirmedX;
}

// Get confirmed y property
float Block::getConfirmedY()
{
  return confirmedY;
}

// Get confirmed z property
float Block::getConfirmedZ()
{
  return confirmedZ;
}

// Set the gotConfirmed property
//   b - the value to assign
void Block::setGotConfirmed(bool b)
{
  gotConfirmed = b;
}

// Get the gotConfirmed property
bool Block::getGotConfirmed()
{
  return gotConfirmed;
}

// Change the blocks properties to match the confirmed properties
//   boundaries - the game area boundaries
//   collisionArray - an array used for collision detection
void Block::toConfirmed(float boundaries[], int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH])
{
  // if it's different in y and the confirmed values are not oldConfirmed
  // or it's grounded
  // then set to confirmed
  //if ( ( (y < confirmedY - 10.0 || y > confirmedY + 5.0) && (confirmedX != oldConfirmedX || confirmedY != oldConfirmedY || confirmedZ != oldConfirmedZ)) || grounded) {
    //if (!grounded) cout << "y: " << y << ", confirmedY: " << confirmedY << endl;
  if (turning || moving) cerr << "Block::toConfirmed: called when block active!!" << endl;
  if (confirmedX != oldConfirmedX || confirmedY != oldConfirmedY || confirmedZ != oldConfirmedZ || grounded) {
    //if (id == 27) cout << "setting to Confirmed: " << id << endl;
    matrixCopy(confirmedMatrix, matrix);
    matrixCopy(confirmedMatrix, oldMatrix);
    x = confirmedX;
    y = confirmedY;
    z = confirmedZ;
    oldX = confirmedX;
    oldY = confirmedY;
    oldZ = confirmedZ;

    oldConfirmedX = confirmedX, oldConfirmedY = confirmedY, oldConfirmedZ = confirmedZ;
    targetX = 0, targetY = 0, targetZ = 0;
    targetAngleX = 0, targetAngleY = 0, targetAngleZ = 0;
    moving = false;
    turning = false;

    // update collision array
    newPosition.clear();
    clearCollisionTrail(collisionArray, false); // clear old position based on lastStored

    lastStored.clear();
    float wx = 0, wy = 0, wz = 0;
    int worldX = 0, worldY = 0, worldZ = 0;
    vector <int> pos;

    // go through block array
    for (int z = 0; z < 5; z++) {
      for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
          if (data[x][y][z] == 1) { // if there's a block here
            // get world coords of current block
            wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
            worldCoords(wx, wy, wz);
            worldX = (int) roundf((wx - boundaries[0]) / 5.0), worldY = (int) roundf(wy / 5.0), worldZ = (int) roundf((wz - boundaries[2]) / 5.0);

            // check we're in range (to stop any annoying segfaults)
            if (worldX < 0 || worldY < 0 || worldZ < 0 ||
                //worldX >= (int) collisionArray.size() || worldY >= (int) collisionArray[0].size() ||
                //worldZ >= (int) collisionArray[0][0].size()) {
                worldX >= GAMEAREA_WIDTH || worldY >= GAMEAREA_HEIGHT ||
                worldZ >= GAMEAREA_DEPTH) {
              cerr << "Block::Block - attempt to check a world coordinate outside of collision array bounds" << endl;
              cerr << "worldX: " << worldX << ", worldY: " << worldY << ", worldZ: " << worldZ << endl;
            }else{
              if (collisionArray[worldX][worldY][worldZ] > 0) {
                // some sort of error?
              }
              collisionArray[worldX][worldY][worldZ] = id;
              pos.clear();
              pos.push_back(worldX);
              pos.push_back(worldY);
              pos.push_back(worldZ);
              lastStored.push_back(pos);
            }
          }
        }
      }
    }

    // we've put the block in the collision array, so size of lastStored should
    // be safelyStoredSize
    if (safelyStoredSize != (int) lastStored.size()) {
      cerr << "Block::toConfirmed(): lastStored.size() != safelyStoredSize" << endl;
      cerr << "lastStored.size() == " << lastStored.size() << ", safelyStoredSize == " << safelyStoredSize << endl;
    }

    gotConfirmed = false;

    //if (matrix[0] > 0.9 && matrix[0] < 1.1) {
    /*  cout << "matrix for " << id;
      for (int i = 0; i < 16; i++) {
        if (i%4 == 0) cout << endl;
        cout << matrix[i];
      }
      cout << endl;
      */
    //}

    // NOTE does not set grounded = true here... maybe it should?
  }
}

// checks for collision against other blocks and world boundaries
// Expected action is to call hit() as a result of collision
//   blocks - vector storing all the game blocks
//   boundaries - the game area boundaries
//
// Returns:
//   true if hit, false otherwise
bool Block::originalCheckCollision(vector <Block> &blocks, float boundaries[])
{
  // variables to store world coordinate of current block
  // and block you're checking against
  float wx = 0, wy = 0, wz = 0; // current
  float wx2 = 0, wy2 = 0, wz2 = 0; // other

  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (data[x][y][z] == 1) { // if there's a block here
          // get world coords of current block
          wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
          worldCoords(wx, wy, wz);

          // when checking points were equal, used to round
          //wx = roundf(wx), wy = roundf(wy), wz = roundf(wz);

          // has it hit the ground whilst turning?
          // can't do < 0 because blocks actually turn through ground as they rotate
          if (wy < -0.2) return true;

          // check against X and Z boundaries (accounting for rounding errors)
          if (wx < boundaries[0] - 0.2 || wx > boundaries[1] + 0.2 || wz < boundaries[2] - 0.2 || wz > boundaries[3] + 0.2)
            return true;

          // go through all the others
          for (int i = 0; i < (int) blocks.size(); i++) {

            if (id != blocks[i].getId()) { // don't check collision against itself!

              // loop for every bit of block data to check for collision
              for (int z2 = 0; z2 < 5; z2++) {
                for (int y2 = 0; y2 < 5; y2++) {
                  for (int x2 = 0; x2 < 5; x2++) {
                    if (blocks[i].getData(x2, y2, z2) == 1) { // if there's a block here
                      wx2 = x2 * 5.0, wy2 = y2 * 5.0, wz2 = z2 * 5.0; // object coords
                      blocks[i].worldCoords(wx2, wy2, wz2); // convert to world coords
                      // when checking points were equal, used to round
                      //wx2 = roundf(wx2), wy2 = roundf(wy2), wz2 = roundf(wz2);
                      //if (wx == wx2 && wy == wy2 && wz == wz2) {
                      // if within a block width of each other
                      if (wx < wx2 + 4.9 && wx > wx2 - 4.9 && wy < wy2 + 4.9 && wy > wy2 - 4.9 && wz < wz2 + 4.9
                          && wz > wz2 - 4.9) {
                        //cout << collision++ << endl;
                        // special case - block moving down has collided with one above it
                        // if it's below, and not moving up then it's clear
                        // so check... if it's above or equal, or it's moving up then it's hit
                        if (wy > wy2 - 0.1 || targetY >= y) return true;
                      }
                    }
                  } // x
                } // y
              } // z

            } // end not the same block as current one

          } // end for going through blocks
        } // end if there's a block here for current block
      } // x
    } // y
  } // z

  return false;
}

// A collision detection routine using the collision array
//   collisionArray - the array used for collision detection
//   boundaries - the game boundaries
//
// Returns:
//   true if hit, false otherwise
bool Block::checkCollision(int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float boundaries[])
{
  float wx = 0, wy = 0, wz = 0; // world coordinates of block
  float checkX = 0, checkY = 0, checkZ = 0; // check coordinates (for checking ahead of movement)
  int worldX = 0, worldY = 0, worldZ = 0; // rounded world coords
  bool hit = false, alreadyStored = false;
  vector <int> pos; // temporary position vector
  newPosition.clear();

  // remove last stored position from collisionArray
  // this will also mean it won't check against itself

  // block will not check against itself because it uses the id in
  // collisionArray

  //for (int i = 0; i < (int) lastStored.size(); i++) {
  //  collisionArray[lastStored[i][0]][lastStored[i][1]][lastStored[i][2]] = 0;
  //}
  
  // go through block array
  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (data[x][y][z] == 1 && !hit) { // if there's a block here
          // get world coords of current block
          wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
          worldCoords(wx, wy, wz);

          // has it hit the ground whilst turning?
          // can't do < 0 because blocks actually turn through ground as they rotate
          if (wy < -0.2) hit = true;

          // check against X and Z boundaries (accounting for rounding errors)
          if (wx < boundaries[0] - 0.2 || wx > boundaries[1] + 0.2 || wz < boundaries[2] - 0.2 || wz > boundaries[3] + 0.2) {
            hit = true;
            wallMark[0] = wx, wallMark[1] = wy, wallMark[2] = wz;
            wallMark[3] = 0; // don't rotate
            if (wx < boundaries[0] - 0.2) wallMark[3] = 3;
            if (wz < boundaries[2] - 0.2) wallMark[3] = 2;
            if (wx > boundaries[1] + 0.2) wallMark[3] = 1;
            if (wz > boundaries[3] + 0.2) wallMark[3] = 0;
            wallMarkAlpha = 1.0;
          }
  
          // check against collisionArray, in all directions except up
          for (int i = 0; i < 5; i++) {
            checkX = wx, checkY = wy, checkZ = wz;

            // check 2.3 either side (because move 0.1 so that makes a total of 2.4 in direction of movement)
            // 2.5 would enter next block even if it was staying still
            switch (i) {
              case 0:
                checkX = wx - 2.2;
                break;
              case 1:
                checkX = wx + 2.2;
                break;
              case 2:
                checkZ = wz - 2.2;
                break;
              case 3:
                checkZ = wz + 2.2;
                break;
              case 4:
                checkY = wy - 2.2;
                break;
            }

            // get array coordinates from check coordinates
            worldX = (int) roundf((checkX - boundaries[0]) / 5.0), worldY = (int) roundf(checkY / 5.0), worldZ = (int) roundf((checkZ - boundaries[2]) / 5.0);
            //cout << "[0]: " << collisionArray[0].size() << ", wy: " << worldY << endl;
            //cout << "wx: " << wx << ", wy: " << wy << ", wz: " << wz << endl;
            //cout << "worldX: " << worldX << ", worldY: " << worldY << ", worldZ: " << worldZ << endl;

            if (worldX < 0 || worldY < 0 || worldZ < 0 ||
                worldX >= GAMEAREA_WIDTH || worldY >= GAMEAREA_HEIGHT ||
                worldZ >= GAMEAREA_DEPTH) {
              //cerr << "Block::checkCollision - attempt to check a world coordinate outside of collision array bounds" << endl;
              //cerr << "worldX: " << worldX << ", worldY: " << worldY << ", worldZ: " << worldZ << endl;
              // could hit too high (13) or x == 5 cos not picked up by boundary check (because checkX is not checked
              // against boundary, only wx is)
            }else
              if (collisionArray[worldX][worldY][worldZ] > 0 && collisionArray[worldX][worldY][worldZ] != id) hit = true;
          } // end for check points
            
          if (!hit) {
            // store position of block, for entry into collisionArray
            worldX = (int) roundf((wx - boundaries[0]) / 5.0), worldY = (int) roundf(wy / 5.0), worldZ = (int) roundf((wz - boundaries[2]) / 5.0);

            pos.clear();
            pos.push_back(worldX);
            pos.push_back(worldY);
            pos.push_back(worldZ);
            newPosition.push_back(pos);
          }

        } // end if there is a block and !hit
      } // x
    } // y
  } // z

  if (!hit) { // we're clear to move to new position
    // so add new stored positions to collision array/lastStored
    for (int i = 0; i < (int) newPosition.size(); i++) {
      collisionArray[newPosition[i][0]][newPosition[i][1]][newPosition[i][2]] = id;

      // is this position already stored?
      alreadyStored = false;
      for (int j = 0; j < (int) lastStored.size(); j++) {
        if (lastStored[j][0] == newPosition[i][0] && lastStored[j][1] == newPosition[i][1] && lastStored[j][2] == newPosition[i][2]) alreadyStored = true;
      }
      // if not add it
      if (!alreadyStored) lastStored.push_back(newPosition[i]);
    }

    // new position now contains only the new position vectors
  }else{
    // reset to safely stored original position

    // firstly remove all stored positions from the array that were not in the
    // original part
    for (int i = safelyStoredSize; i < (int) lastStored.size(); i++)
      collisionArray[lastStored[i][0]][lastStored[i][1]][lastStored[i][2]] = 0;

    // secondly remove those stored positions from the array
    while ((int) lastStored.size() > safelyStoredSize) lastStored.pop_back();
  }

  return hit; // collision?
}

// Clear the collision trail
//   collisionArray - the array storing collision data
//   storeNewPosition - whether or not to store new block position
void Block::clearCollisionTrail(int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], bool storeNewPosition)
{
  // clear lastStored and set newPosition
  for (int i = 0; i < (int) lastStored.size(); i++)
    collisionArray[lastStored[i][0]][lastStored[i][1]][lastStored[i][2]] = 0;
  
  if (storeNewPosition) {
    lastStored = newPosition;
    if ((int) lastStored.size() != safelyStoredSize) {
      cerr << "Block::clearCollisionTrail: lastStored.size() != safelyStoredSize" << endl;
      cerr << "lastStored.size() == " << lastStored.size() << ", safelyStoredSize == " << safelyStoredSize << endl;
    } else {
      for (int i = 0; i < (int) lastStored.size(); i++)
        collisionArray[lastStored[i][0]][lastStored[i][1]][lastStored[i][2]] = id;
    }
  }
}

// Remove a layer of cubes from the block.
//   blocks - the global store of game blocks
//   outputBlocks - new 'split' blocks are stored in this
//   collisionArray - the array used for collisionDetection
//   boundaries - the game boundaries
//   blockId - the block id number
//   layerY - the layer being removed
//
// Returns:
//   true if a layer has been removed (and therefore this block can be deleted)
//   false otherwise
bool Block::removeLayer(vector <Block> blocks, vector <Block> &outputBlocks, int collisionArray[GAMEAREA_WIDTH][GAMEAREA_HEIGHT][GAMEAREA_DEPTH], float boundaries[], int &blockId, int layerY)
{
  // outputBlocks is appended with the new split blocks, or is left alone

  float wx = 0, wy = 0, wz = 0; // world coords
  int worldX, worldY, worldZ; // rounded world coords for collision array
  bool broken = false;
  
  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (data[x][y][z] == 1) { // if there's a block here
          // get world coords of current block
          wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
          worldCoords(wx, wy, wz);

          if (wy > layerY - 0.1 && wy < layerY + 0.1) { // does it match layerY
            data[x][y][z] = 0;
            interactive = false; // block is now broken into pieces and cannot be manipulated
            broken = true;
            worldX = (int) roundf((wx - boundaries[0]) / 5.0), worldY = (int) roundf(wy / 5.0), worldZ = (int) roundf((wz - boundaries[2]) / 5.0);
            if (worldX < 0 || worldY < 0 || worldZ < 0 ||
                //worldX >= (int) collisionArray.size() || worldY >= (int) collisionArray[0].size() ||
                //worldZ >= (int) collisionArray[0][0].size()) {
                worldX >= GAMEAREA_WIDTH || worldY >= GAMEAREA_HEIGHT ||
                worldZ >= GAMEAREA_DEPTH) {
              cerr << "Block::removeLayer - attempt to check a world coordinate outside of collision array bounds" << endl;
              cerr << "worldX: " << worldX << ", worldY: " << worldY << ", worldZ: " << worldZ << endl;
            }else
              collisionArray[worldX][worldY][worldZ] = 0;
          }
        }
      } // end x
    } // end y
  } // end z

  if (broken) { // only split if it's just been broken (to avoid recursion)
    clearCollisionTrail(collisionArray, false);
    // split block into individual pieces
    for (int z = 0; z < 5; z++) {
      for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
          if (data[x][y][z] == 1) { // if there is a block here
            // get world coords of current block
            wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
            worldCoords(wx, wy, wz);

            //cout << "about to add a block" << endl;
            // add the new split blocks to the output vector
            outputBlocks.push_back(Block(blockId++, 0, roundf(wx), roundf(wy), roundf(wz), false, collisionArray, boundaries));
            // new blocks are not grounded by default
            //cout << "block added" << endl;
          }
        } // end x
      } // end y
    } // end z

    return true; // notify that a layer has been removed from this block and it should not be added to outputBlocks
  }

  return false; // continue as normal
}

// Return vectors containing x and z positions at a particular layer
//   layerY - the y coordinate of the layer
vector <vector <float> > Block::getLayer(int layerY)
{
  vector <float> temp;
  vector <vector <float> > layer; // all the position vectors
  
  float wx = 0, wy = 0, wz = 0; // world coords
  
  for (int z = 0; z < 5; z++) {
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        if (getData(x, y, z) == 1) { // if there's a block here
          // get world coords of current block
          wx = x * 5.0, wy = y * 5.0, wz = z * 5.0;
          worldCoords(wx, wy, wz);

          if (wy > layerY - 0.1 && wy < layerY + 0.1) { // does it match layerY
            temp.clear();
            temp.push_back(wx);
            temp.push_back(wy);
            temp.push_back(wz);

            layer.push_back(temp);
          }
        }
      } // end x
    } // end y
  } // end z

  return layer;
}

// Called when a collision takes place
void Block::hit()
{
  //cout << "oldX,Y,Z: " << oldX << ", " << oldY << ", " << oldZ << endl;
  if (y > targetY && hitCount < 5) hitCount++; // if moving down
  else hitCount = 0; // otherwise reset
  
  // must not be equal to above condition i.e. conditions must cover all posibilities
  if (hitCount > 4) grounded = true;
  // blocks become ungrounded in removeLayer when they are split
  // or when they are manipulated or when a layer is complete (in cve.cc)

  x = oldX, y = oldY, z = oldZ;
  matrixCopy(oldMatrix, matrix);
  targetX = 0, targetY = 0, targetZ = 0;
  targetAngleX = 0, targetAngleY = 0, targetAngleZ = 0;
  moving = false;
  turning = false;
  //cout << "hit" << endl;
}

// called when block is manipulated to unground the block and reset the hit count
void Block::unGrounded()
{
  hitCount = 0;
  grounded = false;
}

