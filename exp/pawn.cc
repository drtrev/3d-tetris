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

#include "pawn.h"

using namespace std;

Pawn::Pawn()
{
  id = 0;
  angleX = 0.0, angleY = 0.0, angleZ = 0.0, turnSpeed = 0.6 * GAME_SPEED;
  x = 0.0, y = 0.0, z = 0.0;
  ground = 0, height = 24, ceil = height;
  speedX = 0.0, speedY = 0.0, speedZ = 0.0;
  pushX = 0.0, pushY = 0.0, pushZ = 0.0;
  speed = 0.0, accel = 0.04 * GAME_SPEED, maxSpeed = 1.0 * GAME_SPEED;
  strafeRightSpeed = 0.0; // for strafe left use negative speed
  resistance = 0.02 * GAME_SPEED;
  friction = 0.02 * GAME_SPEED;
  pointHeight.clear();
  pointHeight.push_back(0);
  pointHeight.push_back(0);
  pointHeight.push_back(0);
  pointHeight.push_back(0);
}

float Pawn::getX() const
{
  return x;
}

float Pawn::getY() const
{
  return y;
}

float Pawn::getZ() const
{
  return z;
}

float Pawn::getAngleX() const
{
  return angleX;
}

float Pawn::getAngleY() const
{
  return angleY;
}

float Pawn::getAngleZ() const
{
  return angleZ;
}

float Pawn::getSpeed() const
{
  return speed;
}

float Pawn::getStrafeRightSpeed() const
{
  return strafeRightSpeed;
}

float Pawn::getMaxSpeed() const
{
  return maxSpeed;
}

float Pawn::getAccel() const
{
  return accel;
}

void Pawn::setX( const float nx )
{
  x = nx;
}

void Pawn::setY( const float ny )
{
  y = ny;
}

void Pawn::setZ( const float nz )
{
  z = nz;
}

void Pawn::setGround( const float gr )
{
  ground = gr;
  for (int i = 0; i < 4; i++) pointHeight[i] = ground;
}

void Pawn::setCeil( const float cl )
{
  ceil = cl;
}

void Pawn::setAngleX( const float nAngleX )
{
  angleX = nAngleX;
  loopAngles();
}

void Pawn::setAngleY( const float nAngleY )
{ // WARNING: does not lock it to forward angles (can look upside-down)
  // this is because for player controls turn() is used
  angleY = nAngleY;
  loopAngles();
}

void Pawn::setAngleZ( const float nAngleZ )
{
  angleZ = nAngleZ;
  loopAngles();
}

void Pawn::setSpeed( const float ns )
{
  speed = ns;
}

void Pawn::setStrafeRightSpeed( const float ns )
{
  strafeRightSpeed = ns;
}

void Pawn::changeGround( const float amount, const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  float oldGround = ground, oldY = y;
  ground += (amount * timeSecs / SECS_PER_FRAME);

  // change pointHeights to match
  for (int i = 0; i < 4; i++) {
    pointHeight[i] = ground;
  }
  y = ground; // and y

  lockPosition(x, oldY, z, oldGround);
}

void Pawn::accelerate( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  if (speed < maxSpeed) speed += (accel * timeSecs / SECS_PER_FRAME);
  // does not set equal to max speed for smoother accel / decel
}

void Pawn::brake( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  if (speed > -maxSpeed) speed -= (accel * timeSecs / SECS_PER_FRAME);
  // does not set equal to negative max speed for smoother accel / decel
}

void Pawn::strafeRight( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  if (strafeRightSpeed < maxSpeed)
    strafeRightSpeed += (accel * timeSecs / SECS_PER_FRAME);
}

void Pawn::strafeLeft( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  if (strafeRightSpeed > -maxSpeed)
    strafeRightSpeed -= (accel * timeSecs / SECS_PER_FRAME);
}

void Pawn::turnRight()
{
  angleY -= turnSpeed;
  loopAngles();
}

void Pawn::turnLeft()
{
  angleY += turnSpeed;
  loopAngles();
}

void Pawn::turn(float right, float up)
{
  angleY += right;
  angleX += up;
  if (angleX > 90.0) angleX = 90.0; // lock angleX so you can't keep turning upside-down
  if (angleX < -90.0) angleX = -90.0;
  loopAngles();
}

void Pawn::toTarget( float &var, float target, float amount )
{
  if (var < target){
    var += amount;
    if (var > target) var = target;
  }
  if (var > target){
    var -= amount;
    if (var < target) var = target;
  }
}

void Pawn::loopAngles()
{
  if (angleY > 359) angleY -= 360;
  if (angleX > 180) angleX -= 360;
  if (angleZ > 359) angleZ -= 360;
  if (angleY < 0) angleY += 360;
  if (angleX < -180) angleX += 360;
  if (angleZ < 0) angleZ += 360;
}

void Pawn::lockPosition(float oldX, float oldY, float oldZ, float oldGround)
{
  if (x < PLAYER_BOUNDARY_MIN_X || x > PLAYER_BOUNDARY_MAX_X) {
    x = oldX, y = oldY, z = oldZ, ground = oldGround;
    pushX = -speedX, pushY = 0.0, pushZ = 0.0;
  }
  if (y < PLAYER_BOUNDARY_MIN_Y || y > PLAYER_BOUNDARY_MAX_Y) {
    x = oldX, y = oldY, z = oldZ, ground = oldGround;
    pushX = 0.0, pushY = 0.0, pushZ = 0.0;
  }
  if (z < PLAYER_BOUNDARY_MIN_Z || z > PLAYER_BOUNDARY_MAX_Z) {
    x = oldX, y = oldY, z = oldZ, ground = oldGround;
    pushX = 0.0, pushY = 0.0, pushZ = -speedZ;
  }
}

void Pawn::move( const float timeSecs )
//
// timeSecs   IN    Time (s) to be used to calculate movement
//
{
  float oldX = x, oldY = y, oldZ = z;

  speedX = speed * Trig::sn(angleY) + strafeRightSpeed * Trig::sn(angleY-90);
  speedZ = speed * Trig::cs(angleY) + strafeRightSpeed * Trig::cs(angleY-90);
  
  //doFriction(speedX, speedY, speedZ);

  x += speedX * timeSecs / SECS_PER_FRAME + pushX;
  y += speedY * timeSecs / SECS_PER_FRAME + pushY;
  z += speedZ * timeSecs / SECS_PER_FRAME + pushZ;

  distanceMoved += sqrt((speedX + pushX) * (speedX + pushX) + (speedY + pushY) * (speedY + pushY) + (speedZ + pushZ) * (speedZ + pushZ));
  //
  // Not quite sure how this is working, but it seems to be to do with
  // slowing down gradually (RAR)
  //
  toTarget(speed, 0.0, resistance * timeSecs / SECS_PER_FRAME);
  toTarget(strafeRightSpeed, 0.0, resistance * timeSecs / SECS_PER_FRAME);
  toTarget(pushX, 0.0, resistance * timeSecs / SECS_PER_FRAME);
  toTarget(pushY, 0.0, resistance * timeSecs / SECS_PER_FRAME);
  toTarget(pushZ, 0.0, resistance * timeSecs / SECS_PER_FRAME);

  lockPosition(oldX, oldY, oldZ, ground);
}

void Pawn::resetDistanceMoved()
{
  distanceMoved = 0.0;
}

float Pawn::getDistanceMoved()
{
  return distanceMoved;
}

