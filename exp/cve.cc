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
 * CVE - Collaborative Virtual Environment
 *
 * This is the main program for the CVE. The application begins
 * with four levels of training, followed by four single-player
 * games, (the first one slower than normal), followed by four
 * collaborative games.
 *
 * To skip straight to a collaborative game, set the constant
 * SKIP_TO_COLLABORATIVE to 1. The address of the server
 * (running server.cc) is to be given as a command line argument.
 *
 * Please note that the reflection code in this file was inspired
 * by NeHe, http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=26
 * (accessed 12/4/05).
 *
 * Trevor Dodds, 2005
 */

#include <GL/glut.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <deque>
#include <string>
#include "pawn.h"
#include "trig.h"
#include "client.h"
#include "human.h"
#include "texture.h"
#include "block.h"
#include "explosion.h"
#include "game.h"

using namespace std;

// *** NOTE: For evaluation purposes the server increased the rate
//           that new blocks were created. Due to this being a last
//           minute change the server was not programmed to reset
//           its new block timer between games. Instead the server
//           was reset and the 'gameNumber' variable was increased
//           manually.
//
//           The game has been put back to automatically start the
//           next game now and so the rate of creation of new blocks
//           in collaborative games does not increase. See server.cc
//           for details of how to make it suitable for evaluation.
//           To change the rate of increase in single-player games
//           alter NEW_BLOCK_COUNT_DECREASE. This should match the
//           server for evaluation.

// The program is set to play four single player games and then four
// collaborative games. To jump straight into a collaborative game
// set the following to 1. (Otherwise set to 0).

#ifndef START_COLLABORATIVE
#define START_COLLABORATIVE 0
#endif

// Do we want to output information to the standard out, for logfiles?
#define LOG_OUTPUT 0

// do we want to run full screen or in a window?
// if full screen, do we want a high resolution?
#define FULL_SCREEN 1
#define HIGH_RES 1

// set this to a non-zero value to not play the game by the rules!
#define MESS_AROUND 0
// mess around gives you single blocks (but allows forceCornerBlock)
bool forceCornerBlock = false;

int speedCount = 5;

// store the window and it's attributes
int window;
int windowWidth, windowHeight, windowCentreX, windowCentreY; // window details

// store the distance the mouse has moved from the centre of the window
int mouseMoveX = 0, mouseMoveY = 0; //, mouseLastX = 0, mouseLastY = 0;
bool invertMouse = false;
float mouseSensitivity = 0.15, mouseLastAngle = 0.0;

// boolean values to store key presses.
// we don't need to know what the controls are here, we just want to know if the
// user is pressing a key that represents right/left/up etc.
bool pressUp = false, pressDown = false, pressRight = false, pressLeft = false, pressRaise = false, pressLower = false;
bool pressTurnRight = false, pressTurnLeft = false, pressTurnUp = false, pressTurnDown = false;
bool pressLeftButton = false, pressRightButton = false;
bool priorityY = false;
bool mouseLookEnabled = true;
int modifierKeys;
bool output = false, pauseGame = false;
bool warpBack = false;

// How long is the remote connection drawn for?
#define DRAW_CONNECTION_COUNT_MAX 2000
int drawConnectionCount = DRAW_CONNECTION_COUNT_MAX;

// do frustum calculations once then store in this
float rightFrustumBorder;

// how fast is gravity?
// and how often do new blocks appear?
// and what is the speed increase for these timers?
//
// NEW_BLOCK_COUNT_START, NEW_BLOCK_COUNT_MIN and NEW_BLOCK_COUNT_DECREASE
// now defined in game.h (RAR)
//
#define GRAVITY_COUNT_START 2000
#define GRAVITY_COUNT_MIN 600
#define GRAVITY_COUNT_DECREASE 8
// only for slow game

// set up properties for gravity
int gravityTimer = GRAVITY_COUNT_START; // for moving down blocks
bool stopGravity = false;
void gravity(int);

// set up properties for starting new blocks
int blockId = 1, newBlockTimer = NEW_BLOCK_COUNT_START; // for making new blocks appear
bool cancelBlock = false;
void newBlock(int);

// how often do we call the idle function?
// how often do we redraw the screen?
#define TIMER_IDLE 5
#define TIMER_REDISPLAY 5
void idle(int);
void postRedisplay(int);

// define the boundaries for the game area
#define BOUNDARY_MIN_X -10.0
#define BOUNDARY_MAX_X 10.0
#define BOUNDARY_MIN_Z -10.0
#define BOUNDARY_MAX_Z 10.0

// define the starting height of the blocks
#define BLOCK_START_Y 50.0

// how many blocks can fit into the game?
#define GAMEBLOCK_WIDTH ((int) (BOUNDARY_MAX_X - BOUNDARY_MIN_X) / 5 + 1)
#define GAMEBLOCK_DEPTH ((int) (BOUNDARY_MAX_Z - BOUNDARY_MIN_Z) / 5 + 1)
#define GAMEBLOCK_HEIGHT ((int) BLOCK_START_Y / 5 + 1)

// store the boundaries in an array
float boundaries[] = {BOUNDARY_MIN_X, BOUNDARY_MAX_X, BOUNDARY_MIN_Z, BOUNDARY_MAX_Z};

// set up light 0 attributes
GLfloat lightCoords0[] = { 5.0, 44.0, 6.0, 0 };
GLfloat ambient0[] = { 0.3, 0.3, 0.3, 0.3 };
GLfloat diffuse0[] = { 0.6, 0.6, 0.6, 0.5 };
GLfloat specular0[] = { 0.4, 0.4, 0.4, 0.4 };

// set up light 1 attributes
GLfloat lightCoords1[] = { 50.0, 6.0, -10.0, 0 };
GLfloat ambient1[] = { 0.2, 0.2, 0.2, 0.2 };
GLfloat diffuse1[] = { 0.2, 0.2, 0.2, 0.2 };
GLfloat specular1[] = { 0.2, 0.2, 0.2, 0.2 };

GLfloat atmoColor[] = { 0.9, 0.7, 0.3, 1.0 }; // for fog
float texPan = 0.0; // for panning the sky texture

// make a global player object, and a 3D avatar for reflection purposes
Pawn player;
Human me(0, 0, 0, 0, 0, 3, -1);

// a vector to store other players in the game
vector <Human> humans;
int humanFace = 6; // texture of human

// specify the camera height in first person view (i.e. height of player)
float cameraHeight = 0.0;

// a vector to store the blocks in the game
vector <Block> blocks;
int selectedBlock = -1; // id of block selected
int selectedX = 0, selectedY = 0;

// the collision array
//vector <vector <vector <int> > > collisionArray;
int collisionArray[GAMEBLOCK_WIDTH][GAMEBLOCK_HEIGHT+1][GAMEBLOCK_DEPTH];

// explosions occur when a layer is completed
deque <Explosion> explosions;

// set up an instance of Client to look after our network activity
Client client;
bool network = false;
bool gotIpAddress = false;

// which layers have we said we've found?
// which layers have we been told it's safe to remove?
vector <int> sentFoundLayer;
vector <int> receivedRemoveLayer;

// how long is a training level?
// how many training levels are there?
#define TRAINING_TIME 40000
#define TRAINING_LEVELS 4
int trainTimebase = 0;
int training = TRAINING_LEVELS; // counts down to 0 which is normal play

// messages are displayed to explain training levels and notify the user of
// game start/end
// how long should they be displayed for?
#define MESSAGE_COUNT_MAX 1500
string message;
int messageCount = -1; // infinate time for training messages

// how long is the welcome text displayed for?
// at what y coordinate is it displayed?
#define WELCOME_COUNT_MAX 1000
#define TEXT_Y_START 20.0
int welcomeCount = WELCOME_COUNT_MAX;
float textAngleY = 0.0, textY = TEXT_Y_START;

// how long is the game over text displayed for?
// what game numbers require changes in the game type?
#define GAMEOVER_COUNT_MAX 1000
#define GAMENUMBER_SOLO 1
#define GAMENUMBER_FASTGAME 2
#define GAMENUMBER_NETWORK 5
#define GAMENUMBER_END 9
int gameOverCount = GAMEOVER_COUNT_MAX;
bool gameOver = false;
int gameNumber = 0; // increased upon first call to newgame
int gameStartTime = 0, numberOfLayers = 0;

// for frames per second counter
int frame=0,elapsedTime=0,timebase=0;
float fps;
vector <int> fpsStore; // store FPS values

// what is our score?
int score = 0, scoreCount = 0; // score count is used to allow for multiple completed layers
int blockScore = 0; // points for blocks in game area

// for trig operations that don't need to be too accurate
// they can just be looked up from an array
float Trig::cosValues[721];
float Trig::sinValues[721];
float Trig::asinValues[121];

// the texture numbers and filenames
vector <int> Texture::texId;
vector <char*> Texture::filenames;
// this is needed so filenames can be kept and referenced by other functions in texture.h

// Continually sets when the data is ready to send
//   value - the value required by glutTimerFunc()
//           expected to be always 1 (true - ready to send)
void setReadyToSend(int value)
{
  client.setReadyToSend(value);

  glutTimerFunc(100, setReadyToSend, 1);
}

// Clears and resets the three-dimensional collision array (vector)
void clearCollisionArray()
{
  for (int i = 0; i < GAMEBLOCK_WIDTH; i++)
    for (int j = 0; j < GAMEBLOCK_HEIGHT; j++)
      for (int k = 0; k < GAMEBLOCK_DEPTH; k++)
        collisionArray[i][j][k] = 0;
  /*collisionArray.clear();
  vector <int> temp;
  vector <vector <int> > temp2;
  for (int i = 0; i < GAMEBLOCK_DEPTH; i++) temp.push_back(0);
  for (int i = 0; i < GAMEBLOCK_HEIGHT + 1; i++) temp2.push_back(temp); // + 1 because blocks stick up above height
  for (int i = 0; i < GAMEBLOCK_WIDTH; i++) collisionArray.push_back(temp2);
  */
}

// Initialisation function - called once to set up program
void init()
{
  srand ( time(NULL) ); // set the seed for random numbers to the current time

  glClearColor (0.9, 0.7, 0.3, 1.0);

  // enable depth test
  glEnable (GL_DEPTH_TEST);

  // backface culling
  glFrontFace(GL_CCW);
  glEnable (GL_CULL_FACE);
  glCullFace (GL_BACK);

  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);

  // set up lights
  glLightfv (GL_LIGHT0, GL_AMBIENT, ambient0);
  glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuse0);
  glLightfv (GL_LIGHT0, GL_SPECULAR, specular0);

  glLightfv (GL_LIGHT1, GL_AMBIENT, ambient1);
  glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse1);
  glLightfv (GL_LIGHT1, GL_SPECULAR, specular1);

  // enable lights
  glEnable (GL_LIGHT0);
  glEnable (GL_LIGHT1);
  glEnable (GL_LIGHTING);

  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_FOG);
  glFogfv(GL_FOG_COLOR, atmoColor);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 900.0);
  glFogf(GL_FOG_END, 1000.0);

  // ignore key repeat
  glutIgnoreKeyRepeat (1);

  windowWidth = glutGet(GLUT_WINDOW_WIDTH);
  windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
  windowCentreX = windowWidth/2;
  windowCentreY = windowHeight/2;

  glutSetCursor(GLUT_CURSOR_NONE);
  // get rid of title bar on bugged implementations of Mesa
  glutWarpPointer(windowCentreX, windowHeight-1);

  // for frustum
  rightFrustumBorder = 0.1 * (float) windowWidth / (float) windowHeight;

  glMatrixMode (GL_PROJECTION);

  // set up the perspective
  glLoadIdentity();
  glFrustum(-rightFrustumBorder, rightFrustumBorder, -0.1, 0.1, 0.1, 1000.0);

  glMatrixMode (GL_MODELVIEW);

  Trig::init();
  Texture::init();
  Texture::loadTextures();

  player.setY(50);
  player.setGround(50);
  player.setZ(-30);
  player.setAngleX(-20);

  // so that there is no mouseLook movement to begin with
  glutWarpPointer(windowCentreX, windowCentreY);

  clearCollisionArray();

  if (network && !training){
    client.init();
    setReadyToSend(1); // start the readyToSend timer
  }else{
    message = "Training level 1 of 4.\nLook around using the mouse.\nMove around using the 'W', 'A', 'S' and 'D' keys.\n\n";
    message += "These are recommended, but ";
    message += "you can also move\nusing the arrow keys.\n\n(Press Return for next level)";
    trainTimebase = glutGet(GLUT_ELAPSED_TIME);
    blocks.push_back(Block(blockId++, 5, -5, 30, 0, true, collisionArray, boundaries));
    // note blockId is not reset when training is complete
  }

  // for network start gravity based on server call instead to help synchronise gravity
  if (!network && !training) {
    glutTimerFunc(gravityTimer, gravity, 0);
    // make a new block
    newBlock(rand()%8); // this function starts the timer for itself
  }

  if (START_COLLABORATIVE) gameNumber = GAMENUMBER_NETWORK - 1;

  glutTimerFunc(TIMER_IDLE, idle, 0);
  //glutTimerFunc(TIMER_REDISPLAY, postRedisplay, 0);
}

// Draw the ground
void drawGround()
{
  // set up properties
  glColor4f(0.8, 0.8, 0.8, 0.5);
  GLfloat matAmbDiff0[] = { 0.8, 0.8, 0.8, 1.0};
  GLfloat matSpecular0[] = { 1.0, 1.0, 1.0, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDiff0);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular0);
  glMaterialf(GL_FRONT, GL_SHININESS, 50);
  glBindTexture(GL_TEXTURE_2D, Texture::texId[5]);
  
  // draw ground
  glNormal3f(0.0, 1.0, 0.0);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -2.5, 100.0);
  glTexCoord2f(2.0, 0.0); glVertex3f(100.0, -2.5, 100.0);
  glTexCoord2f(2.0, 2.0); glVertex3f(100.0, -2.5, -100.0);
  glTexCoord2f(0.0, 2.0); glVertex3f(-100.0, -2.5, -100.0);
  glEnd();
}

// Draw the sides or 'walls' of the marble cube that marks the game area
void drawWalls()
{
  glColor4f(0.8, 0.8, 0.8, 1.0);
  GLfloat matAmbDiff0[] = { 0.8, 0.8, 0.8, 1.0};
  GLfloat matSpecular0[] = { 0.1, 0.1, 0.1, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDiff0);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular0);
  glMaterialf(GL_FRONT, GL_SHININESS, 0);
  glBindTexture(GL_TEXTURE_2D, Texture::texId[5]);
  
  glBegin(GL_QUADS);
  // left
  glNormal3f(-1.0, 0.0, 0.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, -100.0);
  glTexCoord2f(2.0, 0.0); glVertex3f(-100.0, -100.0, 100.0);
  glTexCoord2f(2.0, 2.0); glVertex3f(-100.0, -2.5, 100.0);
  glTexCoord2f(0.0, 2.0); glVertex3f(-100.0, -2.5, -100.0);

  // right
  glNormal3f(1.0, 0.0, 0.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(100.0, -100.0, 100.0);
  glTexCoord2f(2.0, 0.0); glVertex3f(100.0, -100.0, -100.0);
  glTexCoord2f(2.0, 2.0); glVertex3f(100.0, -2.5, -100.0);
  glTexCoord2f(0.0, 2.0); glVertex3f(100.0, -2.5, 100.0);

  // far
  glNormal3f(0.0, 0.0, -1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(100.0, -100.0, -100.0);
  glTexCoord2f(2.0, 0.0); glVertex3f(-100.0, -100.0, -100.0);
  glTexCoord2f(2.0, 2.0); glVertex3f(-100.0, -2.5, -100.0);
  glTexCoord2f(0.0, 2.0); glVertex3f(100.0, -2.5, -100.0);

  // near
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, 100.0);
  glTexCoord2f(2.0, 0.0); glVertex3f(100.0, -100.0, 100.0);
  glTexCoord2f(2.0, 2.0); glVertex3f(100.0, -2.5, 100.0);
  glTexCoord2f(0.0, 2.0); glVertex3f(-100.0, -2.5, 100.0);
  glEnd();
}

// set up skybox constants
#define SKYBOX_MIN_X -50.0
#define SKYBOX_MAX_X 50.0
#define SKYBOX_MIN_Y -12.0
#define SKYBOX_MAX_Y 15.0
#define SKYBOX_MIN_Z -50.0
#define SKYBOX_MAX_Z 50.0

// Draw the skybox (i.e. the mountains and clouds, which are centered on the player)
void drawSkybox()
{
  glFogfv(GL_FOG_COLOR, atmoColor);  //atmoColor is some array of 4 GLfloats (colour)
  glFogi(GL_FOG_MODE, GL_LINEAR); // Can try GL_EXP
  glFogf(GL_FOG_START, 80.0);
  glFogf(GL_FOG_END, 140.0);

  glPushMatrix();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTranslatef(player.getX(), player.getY(), player.getZ());

  glBindTexture(GL_TEXTURE_2D, Texture::texId[7]);
  glColor3f(0.6, 0.5, 0.0); 
  glBegin(GL_QUADS);
    // floor
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
    glTexCoord2f(1.0, 0.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
    glTexCoord2f(1.0, 1.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
    glTexCoord2f(0.0, 1.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
  glEnd();

  // sky
  // split up into four polygons so they fit within fog range
  glNormal3f(0.0, -1.0, 0.0);
  glColor3f(1.0, 1.0, 1.0);

  glBegin(GL_QUADS);
    glTexCoord2f(texPan, texPan); glVertex3f(220.0, 60.0, 220.0);
    glTexCoord2f(texPan+1.0, texPan); glVertex3f(0.0, 60.0, 220.0);
    glTexCoord2f(texPan+1.0, texPan+1.0); glVertex3f(0.0, 60.0, 0.0);
    glTexCoord2f(texPan, texPan+1.0); glVertex3f(220.0, 60.0, 0.0);

    glTexCoord2f(texPan, texPan); glVertex3f(0.0, 60.0, 220.0);
    glTexCoord2f(texPan+1.0, texPan); glVertex3f(-220.0, 60.0, 220.0);
    glTexCoord2f(texPan+1.0, texPan+1.0); glVertex3f(-220.0, 60.0, 0.0);
    glTexCoord2f(texPan, texPan+1.0); glVertex3f(0.0, 60.0, 0.0);

    glTexCoord2f(texPan, texPan); glVertex3f(0.0, 60.0, 0.0);
    glTexCoord2f(texPan+1.0, texPan); glVertex3f(-220.0, 60.0, 0.0);
    glTexCoord2f(texPan+1.0, texPan+1.0); glVertex3f(-220.0, 60.0, -220.0);
    glTexCoord2f(texPan, texPan+1.0); glVertex3f(0.0, 60.0, -220.0);

    glTexCoord2f(texPan, texPan); glVertex3f(220.0, 60.0, 0.0);
    glTexCoord2f(texPan+1.0, texPan); glVertex3f(0.0, 60.0, 0.0);
    glTexCoord2f(texPan+1.0, texPan+1.0); glVertex3f(0.0, 60.0, -220.0);
    glTexCoord2f(texPan, texPan+1.0); glVertex3f(220.0, 60.0, -220.0);
  glEnd(); 

  texPan += 0.00001;
  if (texPan > 1.0) texPan -= 1.0;

  // draw background walls i.e. mountains
  glBindTexture(GL_TEXTURE_2D, Texture::texId[8]);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBegin(GL_QUADS);
   //glNormal3f(0.0, 1.0, 0.0);
   glColor3f(0.6, 0.3, 0.0); 
   glTexCoord2f(0.0, 0.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
   glTexCoord2f(1.0, 0.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
   glTexCoord2f(1.0, 1.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MAX_Y, SKYBOX_MIN_Z);
   glTexCoord2f(0.0, 1.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MAX_Y, SKYBOX_MIN_Z);

   //glNormal3f(-1.0, 1.0, 0.0);
   glTexCoord2f(0.0, 0.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
   glTexCoord2f(1.0, 0.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
   glTexCoord2f(1.0, 1.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MAX_Y, SKYBOX_MAX_Z);
   glTexCoord2f(0.0, 1.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MAX_Y, SKYBOX_MIN_Z);

   //glNormal3f(0.0, 1.0, -1.0);
   glTexCoord2f(0.0, 0.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
   glTexCoord2f(1.0, 0.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
   glTexCoord2f(1.0, 1.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MAX_Y, SKYBOX_MAX_Z);
   glTexCoord2f(0.0, 1.0); glVertex3f(SKYBOX_MAX_X, SKYBOX_MAX_Y, SKYBOX_MAX_Z);

   //glNormal3f(1.0, 1.0, 0.0);
   glTexCoord2f(0.0, 0.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MAX_Z);
   glTexCoord2f(1.0, 0.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MIN_Y, SKYBOX_MIN_Z);
   glTexCoord2f(1.0, 1.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MAX_Y, SKYBOX_MIN_Z);
   glTexCoord2f(0.0, 1.0); glVertex3f(SKYBOX_MIN_X, SKYBOX_MAX_Y, SKYBOX_MAX_Z);

  glEnd();

  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING); 
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glFogfv(GL_FOG_COLOR, atmoColor);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 900.0);
  glFogf(GL_FOG_END, 1000.0);
}

// Draw the game blocks
void drawBlocks()
{
  for (int i = 0; i < (int) blocks.size(); i++) {
    if (i != selectedBlock) { // draw selected block on top
      glLoadName(i);
      blocks[i].draw(Texture::texId, false);
    }
  }
  if (selectedBlock > -1) { // this way arrows appear on top
    glLoadName(selectedBlock);
    blocks[selectedBlock].draw(Texture::texId, true);
  }
}

// Draw the game boundaries
void drawBoundaries()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);
  glDisable(GL_TEXTURE_2D);

  glColor4f(1.0, 1.0, 1.0, 0.4);
  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, -1.0);
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[1]+2.5, BLOCK_START_Y + 2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[0]-2.5, BLOCK_START_Y + 2.5, boundaries[2]-2.5);

  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[0]-2.5, BLOCK_START_Y + 2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, BLOCK_START_Y + 2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[3]+2.5);

  glNormal3f(1.0, 0.0, 0.0);
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[0]-2.5, BLOCK_START_Y + 2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[0]-2.5, BLOCK_START_Y + 2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[3]+2.5);

  glNormal3f(-1.0, 0.0, 0.0);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, BLOCK_START_Y + 2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, BLOCK_START_Y + 2.5, boundaries[2]-2.5);
  glEnd();

  glDisable(GL_DEPTH_TEST);
  glColor4f(0.8, 0.4, 0.4, 0.4);
  glBegin(GL_QUADS);
  // floor - note this is drawn in the same place twice due to reflection
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[3]+2.5);
  glVertex3f(boundaries[1]+2.5, -2.5, boundaries[2]-2.5);
  glVertex3f(boundaries[0]-2.5, -2.5, boundaries[2]-2.5);
  glEnd();
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

// Draw the collision array - used for testing purposes
void drawCollisionArray()
{
  glDisable(GL_TEXTURE_2D);
  glColor4f(0.8, 0.8, 0.8, 1.0);
  /*for (int z = 0; z < (int) collisionArray[0][0].size(); z++) {
    for (int y = 0; y < (int) collisionArray[0].size(); y++) {
      for (int x = 0; x < (int) collisionArray.size(); x++) {
        if (collisionArray[x][y][z] > 0) {
          glPushMatrix();
          glTranslatef(boundaries[0] + x*5, y*5, boundaries[2] + z*5);
          glutSolidCube(6.0);
          glPopMatrix();
        }
      }
    }
  }*/
  glEnable(GL_TEXTURE_2D);
}

// Display text on the screen
//
// str - the string of text to be displayed
// x, y - the coordinates for the text
void displayText(string str, float x, float y)
{
  float initX = x;
  glPushMatrix();
	glLoadIdentity();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  for(unsigned int s = 0; s < str.length(); s++)
  {
    if (str[s] == '\n') {
      x = initX;
      y += 20.0;
    }else{
      glRasterPos2f(x, y);
      //glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[s]);
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[s]);
      x += 10.5;
    }
  }
  glPopMatrix();
}

// Draw a connection from the player to the selected block
void drawLocalConnection()
{
  // the three points of the triangle
  float x1 = 0, y1 = 0, z1 = 0;
  float x2 = 0, y2 = 0, z2 = 0;
  float x3 = 0, y3 = 0, z3 = 0;
  if (selectedBlock > -1) {
    blocks[selectedBlock].getPivotCoords(x1, y1, z1);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glDisable(GL_CULL_FACE);
      glColor4f(0.2, 0.7, 0.2, 0.5);

      // pin triangle to player
      x2 = -0.5, y2 = -1.0, z2 = 0.0;

      // rotate vector about Y axis
      float radAngleY = -player.getAngleY() / 180.0 * 3.1416;
      float cosRadY = cos(radAngleY);
      float sinRadY = sin(radAngleY);
      float nx2 = x2 * cosRadY - z2 * sinRadY;
      z2 = z2 * cosRadY + x2 * sinRadY;
      x2 = nx2;
      // y here should match y above
      x3 = -x2, y3 = -1.0, z3 = -z2;

      x2 += player.getX(), y2 += player.getY(), z2 += player.getZ();
      x3 += player.getX(), y3 += player.getY(), z3 += player.getZ();

      // add a negative z vector to make it so when you look down you don't see a gap in the line
      float x4 = 0.0, z4 = -1.0;
      float nx4 = x4 * cosRadY - z4 * sinRadY;
      z4 = z4 * cosRadY + x4 * sinRadY;
      x4 = nx4;

      x2 += x4, z2 += z4;
      x3 += x4, z3 += z4;

      glNormal3f(0.0, 1.0, 0.0);
      glBegin(GL_TRIANGLES);
      glVertex3f(x2, y2, z2);
      glVertex3f(x3, y3, z3);
      glVertex3f(x1, y1, z1);
      glEnd();
      
      glDisable(GL_BLEND);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
  }
}

// Draw the connection from another user (not us) to their selected block
// 
// blockNum - the number of the block that they have selected
// humanNum - the number of the human (player) which has selected it
void drawRemoteConnection(int blockNum, int humanNum)
{
  // block to human axis
  float x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0;

  if (humanNum < 0 || humanNum > (int) humans.size() - 1){
    cerr << "drawConnection: humanNum out of range: " << humanNum << endl;
  }else if (blockNum < 0 || blockNum > (int) blocks.size() -1) {
    // we can output things here for testing purposes
    // 
    //cerr << "drawConnection: blockNum out of range: " << blockNum << endl;
    //cerr << "...assuming it's just because it's not appeared yet or because blocks have split" << endl;
  }else if (drawConnectionCount > 0){
    
    blocks[blockNum].getPivotCoords(x1, y1, z1);
    x2 = humans[humanNum].getX();
    y2 = humans[humanNum].getY();
    z2 = humans[humanNum].getZ();

    glPushMatrix();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.8, 0.2, 0.2, 0.6);

    glNormal3f(0.0, 0.0, 1.0);
    glLineWidth(30);
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
    glLineWidth(1);
    
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    
    drawConnectionCount--;
  }
}

// Map a texture containing text onto a cylinder
//   texture - the id of the texture to use
void display3DText(int texture)
{
  glPushMatrix();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glTranslatef(0.0, textY, 0.0);
  glRotatef(textAngleY, 0.0, 1.0, 0.0);
  glRotatef(-90.0, 1.0, 0.0, 0.0);
  glColor4f(1.0, 1.0, 1.0, 0.5);
  glBindTexture(GL_TEXTURE_2D, Texture::texId[texture]);
  GLUquadricObj* obj = gluNewQuadric();
  gluQuadricTexture(obj, GL_TRUE);
  gluCylinder(obj, 20.0, 20.0, 20.0, 32, 1);
  glDisable(GL_BLEND);
  glPopMatrix();
}

// Called to redraw all the graphics
void display()
{
  // the render mode could be GL_RENDER for drawing the scene or GL_SELECT for picking
  // (selecting an object with the mouse)
  GLint renderMode;
  glGetIntegerv(GL_RENDER_MODE, &renderMode);

  double clipEq[] = {0.0f, -1.0f, 0.0f, 0.0f}; // for reflections
  int locked = 0;

  if (renderMode == GL_RENDER){
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode (GL_PROJECTION);

    // set up the perspective
    glLoadIdentity();
    glFrustum(-rightFrustumBorder, rightFrustumBorder, -0.1, 0.1, 0.1, 1000.0);

    glMatrixMode (GL_MODELVIEW);
  }

  glLoadIdentity();

  // first of all rotate to correct angle
  glRotatef(-player.getAngleX(), 1.0, 0.0, 0.0);
  glRotatef(-player.getAngleY()+180.0, 0.0, 1.0, 0.0);

  // then translate the camera into position
  glTranslatef(-player.getX(), -player.getY() - cameraHeight, -player.getZ());

  if (renderMode == GL_RENDER){
    // Reflection code inspired by nehe tutorial
    // http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=26
    // (accessed 12/4/05)

    //glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE); // don't draw anything to the screen
    // draw ground twice to make it normal colour again (50% opacity)

    drawSkybox();
    // note that the window isn't initialised with GLUT_STENCIL because it causes slow down.
    // The sides of the marble cube cover any block reflections but the avatar reflection
    // can be seen!
    glEnable(GL_STENCIL_TEST);                  // Enable Stencil Buffer For "marking" The Floor
    glStencilFunc(GL_ALWAYS, 1, 1);             // Always Passes, 1 Bit Plane, 1 As Mask
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // We Set The Stencil Buffer To 1 Where We Draw Any Polygon

    glDisable(GL_DEPTH_TEST); // so that we can ignore second parameter of glStencilOp
    drawGround();
    glEnable(GL_DEPTH_TEST);

    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // draw things again
    glStencilFunc(GL_EQUAL, 1, 1);						// We Draw Only Where The Stencil Is 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);   // Don't Change The Stencil Buffer

    glPushMatrix();
    glTranslatef(0.0, -2.5, 0.0); // translate down to ground level
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, clipEq);
      glTranslatef(0.0, -2.5, 0.0); // translate down to ground level

      glScalef(1.0, -1.0, 1.0);

      // put lights into position
      glLightfv (GL_LIGHT0, GL_POSITION, lightCoords0);
      glLightfv (GL_LIGHT1, GL_POSITION, lightCoords1);

      glCullFace(GL_FRONT);
      drawBlocks();
      //glTranslatef(0.0, 5, 0.0); // translate down to ground level
      if (welcomeCount > 0) display3DText(0);
      if (gameOver) display3DText(1);
      for (int i = 0; i < (int) humans.size(); i++) {
        locked = humans[i].getLocked();
        if (locked > -1) {
          drawRemoteConnection(locked, i);
        }
      }
      //drawBoundaries(); -- causes z-fighting cos the ones near camera are drawn
      for (int i = 0; i < (int) humans.size(); i++) humans[i].draw(Texture::texId, 6);
      glTranslatef(player.getX(), player.getY() + 10.0, player.getZ());
      glRotatef(player.getAngleY(), 0.0, 1.0, 0.0);
      glRotatef(-player.getAngleX(), 1.0, 0.0, 0.0);
      me.draw(Texture::texId, 6);
      glCullFace(GL_BACK);
    glPopMatrix();
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_STENCIL_TEST);

    // put lights into position again (this time without being mirrored in y)
    glLightfv (GL_LIGHT0, GL_POSITION, lightCoords0);
    glLightfv (GL_LIGHT1, GL_POSITION, lightCoords1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawGround();
    glDisable(GL_BLEND);
    drawWalls();

    drawBoundaries();

    for (int i = 0; i < (int) humans.size(); i++) {
      humans[i].draw(Texture::texId, 6); // was third parameter of i+1 now humans hold their face texture
    }
    //if (humans.size() > 1) cout << "in cve.cc humans.size() detected as: " << humans.size() << endl;

  }

  drawBlocks(); // draw real blocks, regardless of whether renderMode is SELECT or RENDER

  if (renderMode == GL_RENDER) {
    for (int i = 0; i < (int) humans.size(); i++) {
      locked = humans[i].getLocked();
      if (locked > -1) {
        drawRemoteConnection(locked, i);
      }
    }
    
    // draw after blocks so the appear on top even though depth mask is disabled
    // note explosion not reflected to save processing power
    for (int i = 0; i < (int) explosions.size(); i++) {
      explosions[i].draw(Texture::texId);
    }
    
    if (output) drawCollisionArray();

    drawLocalConnection();

    if (welcomeCount > 0) display3DText(0);
    if (gameOver) display3DText(1);

    // 2D stuff
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0, windowWidth, 0, -windowHeight );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // draw background to text to make it easier to read
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.2, 0.2, 0.2, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - 120.0, -windowHeight + 5.0);
    glVertex2f(windowWidth - 120.0, -windowHeight + 65.0);
    glVertex2f(windowWidth - 5.0, -windowHeight + 65.0);
    glVertex2f(windowWidth - 5.0, -windowHeight + 5.0);
    glEnd();
    glDisable(GL_BLEND);

    // draw score and FPS
    char str[20];
    sprintf(str, "FPS: %i", (int) fps);
    displayText(str, windowWidth - 115, -windowHeight + 30);
    sprintf(str, "Score: %i", (score * 5 + blockScore * 2));
    displayText(str, windowWidth - 115, -windowHeight + 50); // was -windowHeight + 30 without FPS

    // should we be displaying a message box?
    if (messageCount > 0 || messageCount == -1) {
      // display message
      float messageX = 0.0, messageY = 0.0, tempX = 0.0;
      for(int i = 0; i < (int) message.length(); i++) {
        if (message[i] == '\n') {
          messageY += 20.0;
          if (tempX > messageX) messageX = tempX; // messageX contains longest line
          tempX = 0.0;
        }else tempX += 10.5; // character width
      }

      // in case there is only one line in the message
      if (tempX > messageX) messageX = tempX; // messageX contains longest line

      // make a background polygon so that message is clearer
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(0.2, 0.2, 0.2, 0.5);
      glBegin(GL_QUADS);
      glVertex2f(5.0, -windowHeight + 5.0);
      glVertex2f(5.0, -windowHeight + messageY + 45.0);
      glVertex2f(messageX + 21.0, -windowHeight + messageY + 45.0);
      glVertex2f(messageX + 21.0, -windowHeight + 5.0);
      glEnd();
      glDisable(GL_BLEND);
      displayText(message, 10, -windowHeight + 30);
      glEnable(GL_DEPTH_TEST);
      if (messageCount > 0) messageCount--;
    }

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_LINES); // crosshair
    glVertex2f(windowCentreX-10, -windowCentreY);
    glVertex2f(windowCentreX-5, -windowCentreY);
    glVertex2f(windowCentreX, -windowCentreY-10);
    glVertex2f(windowCentreX, -windowCentreY-5);
    glVertex2f(windowCentreX+5, -windowCentreY);
    glVertex2f(windowCentreX+10, -windowCentreY);
    glVertex2f(windowCentreX, -windowCentreY+5);
    glVertex2f(windowCentreX, -windowCentreY+10);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
  }

  glutSwapBuffers();
}

// Called when window is resized, and also once when window is initialised
//   w - new window width
//   h - new window height
void reshape(int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

// Called when a key is pressed
//   key - the key pressed
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void keyboard(unsigned char key, int x, int y)
{
  // for fps analysing
  //int mean = 0, lowest = 100000, highest = 0;
  
  switch (key) {
    case 27:  // esc
      if (window > -1) {
        glutDestroyWindow(window); 
      }
      if (network && !training && LOG_OUTPUT) {
        cout << "closing network connection - return value: " << client.closeConnection() << endl;
      }
      // output fps values
      /*for (int i = 0; i < (int) fpsStore.size(); i++) {
        cout << fpsStore[i] << endl;
        if (fpsStore[i] < lowest) lowest = fpsStore[i];
        if (fpsStore[i] > highest) highest = fpsStore[i];
        mean += fpsStore[i];
      }
      mean -= lowest; // remove lowest value
      mean -= highest; // remove highest value
      mean /= (int) fpsStore.size() - 2; // mean of the rest
      cout << "Mean: " << mean << endl;*/
      exit(0);
      break;
    case 13: // return
      if (training > 0) trainTimebase = -TRAINING_TIME;
      if (training == 1) {
        blocks.clear();
        selectedBlock = -1;
        // allow game to start
      }
      break;
    case 'w':
      pressUp = true;
      break;
    case 'a':
      pressLeft = true;
      break;
    case 's':
      pressDown = true;
      break;
    case 'd':
      pressRight = true;
      break;
    case 'c':
      pressLower = true;
      break;
    case ';':
      mouseLookEnabled = !mouseLookEnabled;
      pressUp = false, pressDown = false, pressRight = false, pressLeft = false, pressRaise = false, pressLower = false;
      pressTurnRight = false, pressTurnLeft = false, pressTurnUp = false, pressTurnDown = false;
      break;
    case ' ':
      pressRaise = true;
      break;
    case 'f':
      if (selectedBlock > -1 && !blocks[selectedBlock].getTurning())
        blocks[selectedBlock].setTargetAngleX(blocks[selectedBlock].getAngleX()+90);
      break;
    case 'g':
      if (selectedBlock > -1 && !blocks[selectedBlock].getTurning())
        blocks[selectedBlock].setTargetAngleY(blocks[selectedBlock].getAngleY()+90);
      break;
    case 'h':
      if (selectedBlock > -1 && !blocks[selectedBlock].getTurning())
        blocks[selectedBlock].setTargetAngleZ(blocks[selectedBlock].getAngleZ()+90);
      break;
    case 'o':
      //forceCornerBlock = !forceCornerBlock;
      //output = !output; // for testing purposes
      //humans[0].setLocked(1);
      break;
    case 'p':
      //if (selectedBlock > -1) blocks[selectedBlock].output();
      //pauseGame = !pauseGame; // for testing purposes
      break;
    case 'm':
      if (selectedBlock > -1) blocks[selectedBlock].setMode(MODE_GLOBAL_REFERENCE);
      break;
    case 'i':
      if (selectedBlock > -1 && !blocks[selectedBlock].getMoving())
        blocks[selectedBlock].setTargetPosition(blocks[selectedBlock].getX(), blocks[selectedBlock].getY()+5, blocks[selectedBlock].getZ());
      break;
    case 'j':
      if (selectedBlock > -1 && !blocks[selectedBlock].getMoving())
        blocks[selectedBlock].setTargetPosition(blocks[selectedBlock].getX()-5, blocks[selectedBlock].getY(), blocks[selectedBlock].getZ());
      break;
    case 'k':
      if (selectedBlock > -1 && !blocks[selectedBlock].getMoving())
        blocks[selectedBlock].setTargetPosition(blocks[selectedBlock].getX(), blocks[selectedBlock].getY()-5, blocks[selectedBlock].getZ());
      break;
    case 'l':
      if (selectedBlock > -1 && !blocks[selectedBlock].getMoving())
        blocks[selectedBlock].setTargetPosition(blocks[selectedBlock].getX()+5, blocks[selectedBlock].getY(), blocks[selectedBlock].getZ());
      break;
  }
}

// Called when a key is released
//   key - the key released
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void keyboardUp(unsigned char key, int x, int y)
{
  switch (key) {
    case 'w':
      pressUp = false;
      break;
    case 'a':
      pressLeft = false;
      break;
    case 's':
      pressDown = false;
      break;
    case 'd':
      pressRight = false;
      break;
    case 'c':
      pressLower = false;
      break;
    case ' ':
      pressRaise = false;
      break;
    case 'z':
      //rotateAboutZ = false; // used in an older control method
      break;
  }
}

// Called when a 'special' key is pressed e.g. arrow keys
//   key - the key pressed
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void specialKeyPress(int key, int x, int y)
{
  modifierKeys = glutGetModifiers();

  switch (key) {
    case GLUT_KEY_UP:
      if (mouseLookEnabled) pressUp = true;
      else pressTurnUp = true;
      break;
    case GLUT_KEY_DOWN:
      if (mouseLookEnabled) pressDown = true;
      else pressTurnDown = true;
      break;
    case GLUT_KEY_RIGHT:
      if (mouseLookEnabled) pressRight = true;
      else pressTurnRight = true;
      break;
    case GLUT_KEY_LEFT:
      if (mouseLookEnabled) pressLeft = true;
      else pressTurnLeft = true;
      break;
    case GLUT_KEY_HOME:
      pressRaise = true;
      break;
    case GLUT_KEY_END:
      pressLower = true;
      break;
  }
}

// Called when a 'special' key is released e.g. arrow keys
//   key - the key released
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void specialKeyUp(int key, int x, int y)
{
  switch (key) {
    case GLUT_KEY_UP:
      if (mouseLookEnabled) pressUp = false;
      else pressTurnUp = false;
      break;
    case GLUT_KEY_DOWN:
      if (mouseLookEnabled) pressDown = false;
      else pressTurnDown = false;
      break;
    case GLUT_KEY_RIGHT:
      if (mouseLookEnabled) pressRight = false;
      else pressTurnRight = false;
      break;
    case GLUT_KEY_LEFT:
      if (mouseLookEnabled) pressLeft = false;
      else pressTurnLeft = false;
      break;
    case GLUT_KEY_HOME:
      pressRaise = false;
      break;
    case GLUT_KEY_END:
      pressLower = false;
      break;
  }
}

// Deals with block selection by looking up hit records and finding
// the nearest hit. Code inspired from OpenGL: A Primer, E. Angel (2004)
//   hits - the number of hits that took place
//   buffer - an array storing the hit records
void processHits(GLint hits, GLuint buffer[])
{
   GLuint *ptr;
   GLuint nearestDepth, numIds;
   GLint nearestId = -1;

   ptr = buffer; 

   // each time a name is changed or render mode is changed,
   // a hit record is recorded in buffer
   // a record consists of number of names in stack,
   // min depth, max depth, and then the names
   for (int i = 0; i < hits; i++, ptr+=(3+numIds)) {	// for each hit
     numIds = *ptr;

     if( i == 0) { // First hit
       nearestDepth = *(ptr+1);
       nearestId = *(ptr+3);
     }
     else if( *(ptr+1) < nearestDepth ) { // Nearer
       nearestDepth = *(ptr+1);
       nearestId = *(ptr+3);
     }
   }

   if( nearestId > -1) {
     // %u is unsigned decimal integer
     //printf("Nearest object is no. %u\n",nearestId);
     if (nearestId > (int) blocks.size()) {
       cerr << "selectedId too high in process hits" << endl;
     }else if (blocks[nearestId].getInteractive()) {
       blocks[nearestId].unGrounded();
       selectedBlock = nearestId;

       if (network && !training) {
         // send lock data
         char temp = (char) selectedBlock + '0'; // never send null
         string networkData;
         networkData += FLAG_LOCK;
         networkData += temp;

         client.sendData(networkData);
       }
     }
   }
   else {
     selectedBlock = -1; // no selection
   }
}

#define SELECTBUFFER_SIZE 1000

// Called when a mouse button is pressed or released
//   button - the button pressed/released
//   state - the new state of the button i.e. GLUT_UP or GLUT_DOWN
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void mousePress(int button, int state, int x, int y)
{
  // The select buffer is used for storing hit records
  GLuint selectBuf[SELECTBUFFER_SIZE];
  GLint hits;
  GLint viewport[4];

  if ((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) && state == GLUT_DOWN) {
    // pick a block
    glGetIntegerv (GL_VIEWPORT, viewport);

    glSelectBuffer (SELECTBUFFER_SIZE, selectBuf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(0);

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    
    // load the identity matrix and start from scratch,
    // because gluPickMatrix needs to be called first before glFrustum
    glLoadIdentity ();

    // create 5x5 pixel picking region near cursor location
    //gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 5.0, 5.0, viewport);
    gluPickMatrix ((GLdouble) windowCentreX - 3.0, (GLdouble) viewport[3] / 2.0 - 3.0, 5.0, 5.0, viewport);

    glFrustum(-rightFrustumBorder, rightFrustumBorder, -0.1, 0.1, 0.1, 1000.0);

    // render the scene within this viewport - any blocks rendered count as a hit
    glMatrixMode (GL_MODELVIEW);
    display();

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);

    hits = glRenderMode (GL_RENDER);
    processHits (hits, selectBuf);

    if (button == GLUT_LEFT_BUTTON) pressLeftButton = true;
    else pressRightButton = true;

    selectedX = x, selectedY = y; // for one of the manipulation ideas - store original mouse position

    /*if (selectedBlock > -1) {
      float newX = blocks[selectedBlock].getX() + 20.0 * sin(blocks[selectedBlock].getAngleY()/180.0*3.14);
      float newY = blocks[selectedBlock].getY();
      float newZ = blocks[selectedBlock].getZ() + 20.0 * cos(blocks[selectedBlock].getAngleY()/180.0*3.14);
      player.setAngleX(0);
      player.setAngleY(blocks[selectedBlock].getAngleY()-180.0);
      player.setAngleZ(blocks[selectedBlock].getAngleZ());
      player.setX(newX);
      player.setY(newY);
      player.setZ(newZ);
    }*/
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    pressLeftButton = false;
    mouseLastAngle = 0.0;
    //blocks[selectedBlock].snap();
    glutWarpPointer(windowCentreX, windowCentreY);
    //selectedBlock = -1; // deselect
  }
  if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) pressRightButton = false;
}

// This takes x and y the distance the mouse has moved from the centre of the screen
// and stores them in mouseMoveX and mouseMoveY respectively.
// These two variables are used later to make the view change in correspondence to mouse movement.
// mouseLook() sets the pointer back to the middle ready for when the mouse is moved again.
//   x - the x coordinate of the mouse
//   y - the y coordinate of the mouse
void mouseLook(int x, int y)
{
  // if mouse has moved from screen centre
  if (x != windowCentreX || y != windowCentreY) {
    // grab distance mouse has moved
    mouseMoveX = windowCentreX - x;
    mouseMoveY = windowCentreY - y;
    if (invertMouse) mouseMoveY = -mouseMoveY;

    glutWarpPointer(windowCentreX, windowCentreY); // set mouse pointer back to screen centre
  }
}

// Called when mouse is moved and no buttons are being pressed
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void mousePassiveMove(int x, int y)
{
  // if you move the mouse out of the window, pressLeftButton is still set
  // but mouseActiveMove is no longer called, so don't mouseLook
  if (mouseLookEnabled && !pressLeftButton) mouseLook(x, y);
  //mouseLastX = x;
  //mouseLastY = y;
  if (!pressLeftButton) mouseLastAngle = 0.0;
}

// swap two float variables - used in an alternative manipulation system
//   a - a variable to swap
//   b - the variable to swap with
void swap(float &a, float &b)
{
  float temp = a;
  a = b;
  b = temp;
}

// unground everything above a certain coordinate
//   y - the vertical coordinate to unground blocks above
void unGroundAbove(float y)
{
  for (int i = 0; i < (int) blocks.size(); i++) {
    if (blocks[i].getY() > y) blocks[i].unGrounded();
    //else cout << "Checking y: " << y << " against block y: " << blocks[i].getY() << endl;
  }
}

// Called when mouse is moved and buttons are being pressed
//   x - the mouse x coordinate
//   y - the mouse y coordinate
void mouseActiveMove(int x, int y)
{
  string networkData; // data to be sent across network

  // manipulate block
  if (pressLeftButton && selectedBlock > -1) {

    blocks[selectedBlock].unGrounded();
    unGroundAbove(blocks[selectedBlock].getY());
    
    int mx = x - windowCentreX, my = y - windowCentreY;

    int pAngleY = (int) (player.getAngleY()+45) / 90 * 90;
    if (pAngleY == 360) pAngleY = 0;
    
    float angleX = blocks[selectedBlock].getAngleX();
    float angleY = blocks[selectedBlock].getAngleY();
    float angleZ = blocks[selectedBlock].getAngleZ();
    float newAngle = 0.0;

    char temp = (char) selectedBlock + '0';
    networkData += FLAG_MANIPULATE;
    networkData += temp;
    
    // if moving or turning then warp back
    warpBack = true;

    // if not already turning or moving
    if (!blocks[selectedBlock].getTurning() && !blocks[selectedBlock].getMoving()) {
      warpBack = false; // allow user to move
      blocks[selectedBlock].setRemoteMovement(false);

      if (pAngleY == 180) my = -my;

      if (pAngleY == 90 || pAngleY == 270) {
        if (abs(mx) > abs(my) + 1) { // turn right or left
          warpBack = true; // got movement so warp back

          // angleX is -180 to 180
          if (angleZ > 179.0 && angleZ < 179.0) mx = -mx;
          if (angleX > -0.1 && angleX < 0.1 || angleX > 179.0 || angleX < -179.0) {
            if (angleX > 179.0 || angleX < -179.0) mx = -mx;
            if (mx > 0) newAngle = angleY + 90;
            else newAngle = angleY - 90;
            blocks[selectedBlock].setTargetAngleY(newAngle);
            // with global reference angleX,Y,Z == 0
            networkData += 'Y';
          }else{
            if (angleX > 89.0 && angleX < 91.0) mx = -mx;
            if (angleY > 179.0 && angleX < 181.0) mx = -mx;
            if (mx > 0) newAngle = angleX + 90;
            else newAngle = angleX - 90;
            blocks[selectedBlock].setTargetAngleX(newAngle);
            networkData += 'X';
          }

        }else if (abs(mx) < abs(my) - 1) { // turn up or down
          warpBack = true; // got movement so warp back

          if (pAngleY == 90) my = -my;
          if (angleY > 359.0 || angleY < 0.1 || angleY > 179.0 && angleY < 181.0) {
            if (angleY > 179.0 && angleY < 181.0) my = -my;
            if (my < 0) newAngle = angleZ + 90;
            else newAngle = angleZ - 90;
            blocks[selectedBlock].setTargetAngleZ(newAngle);
            networkData += 'Z';
          }else{
            cout << "turning in X" << endl;
            if (angleY > 269.0 && angleY < 271.0) my = -my;
            if (my < 0) newAngle = angleX + 90;
            else newAngle = angleX - 90;
            blocks[selectedBlock].setTargetAngleX(newAngle);
            networkData += 'X';
          }

        }

      }else{
        if (abs(mx) > abs(my) + 1) { // turn right or left
          warpBack = true; // got movement so warp back

          if (mx > 0) newAngle = angleY + 90;
          else newAngle = angleY - 90;
          blocks[selectedBlock].setTargetAngleY(newAngle);
          networkData += 'Y';

        }else if (abs(mx) < abs(my) - 1) { // turn up or down
          warpBack = true; // got movement so warp back

          if (angleY > 359.0 || angleY < 0.1 || angleY > 179.0 && angleY < 181.0) {
            if (angleY > 179.0 && angleY < 181.0) my = -my;
            if (my < 0) newAngle = angleX + 90;
            else newAngle = angleX - 90;
            blocks[selectedBlock].setTargetAngleX(newAngle);
            networkData += 'X';
          }else{
            // this part is not done in global reference
            if (angleY > 269.0 && angleY < 271.0) my = -my;
            if (pAngleY == 270 || pAngleY == 90) my = -my;
            if (my < 0) newAngle = angleX - 90;
            else newAngle = angleX + 90;
            blocks[selectedBlock].setTargetAngleX(newAngle);
            if (my < 0) newAngle = angleY + 90;
            else newAngle = angleY - 90;
            blocks[selectedBlock].setTargetAngleY(newAngle);
            if (my < 0) newAngle = angleZ - 90;
            else newAngle = angleZ + 90;
            blocks[selectedBlock].setTargetAngleZ(newAngle);
          }
        }
      }

      networkData += (newAngle > 0) ? '1' : '0';
      if (networkData.length() == 4) { // axis part will only be put in if we actually turned the block
        if (network && !training) client.sendData(networkData);
        //cout << "(network dependent) sent manipulate info: " << networkData << endl;
      }
    } // end if not turning
      
    if ((mx != 0 || my != 0) && warpBack) glutWarpPointer(windowCentreX, windowCentreY);
  }else if (pressRightButton && selectedBlock > -1) {

    blocks[selectedBlock].unGrounded();
    unGroundAbove(blocks[selectedBlock].getY());
    
    // mx, my, the amount moved x and y
    int mx = x - windowCentreX, my = y - windowCentreY;
    int direction = 0;

    warpBack = true; // if moving or turning

    if (!blocks[selectedBlock].getMoving() && !blocks[selectedBlock].getTurning()) {
      warpBack = false; // allow user to move mouse in a direction
      blocks[selectedBlock].setRemoteMovement(false);

      if ((abs(mx) - abs(my)) > 1 || (abs(my) - abs(mx)) > 1) {
        warpBack = true; // got movement so warp back
                         // This is needed because block may collide and so will not be moving

        if (priorityY) {
          if (abs(mx) > abs(my)) my = 0; // only allow one direction
          else mx = 0;
        }else{
          if (abs(mx) < abs(my)) mx = 0;
          else my = 0;
        }
        if (mx == 0) priorityY = false;
        else priorityY = true;
        // orthogonal priority is varied to allow 'diagonal' zig-zag movement

        // get player angle rounded to nearest 90 degrees
        int pAngleY = (int) (player.getAngleY()+45) / 90 * 90;
        if (pAngleY == 360) pAngleY = 0;

        float moveBlockX = 0.0, moveBlockY = 0.0, moveBlockZ = 0.0;
      
        if (pAngleY == 0) {
          if (mx < 0) moveBlockX = 5.0;
          if (mx > 0) moveBlockX = -5.0;
          if (my < 0) moveBlockZ = 5.0;
          if (my > 0) moveBlockZ = -5.0;
        }
        if (pAngleY == 90) {
          if (mx < 0) moveBlockZ = -5.0;
          if (mx > 0) moveBlockZ = 5.0;
          if (my < 0) moveBlockX = 5.0;
          if (my > 0) moveBlockX = -5.0;
        }
        if (pAngleY == 180) {
          if (mx < 0) moveBlockX = -5.0;
          if (mx > 0) moveBlockX = 5.0;
          if (my < 0) moveBlockZ = -5.0;
          if (my > 0) moveBlockZ = 5.0;
        }
        if (pAngleY == 270) {
          if (mx < 0) moveBlockZ = 5.0;
          if (mx > 0) moveBlockZ = -5.0;
          if (my < 0) moveBlockX = -5.0;
          if (my > 0) moveBlockX = 5.0;
        }

        //if (!network) {
          blocks[selectedBlock].setTargetPosition(blocks[selectedBlock].getX() + moveBlockX,
                  blocks[selectedBlock].getY() + moveBlockY, blocks[selectedBlock].getZ() + moveBlockZ);
        //}

        if (moveBlockX < 0) direction = 0;
        if (moveBlockY < 0) direction = 1;
        if (moveBlockZ < 0) direction = 2;
        if (moveBlockX > 0) direction = 3;
        if (moveBlockY > 0) direction = 4;
        if (moveBlockZ > 0) direction = 5;
        if (network && !training) {
          networkData += FLAG_MOVE;
          char temp = (char) selectedBlock + '0';
          networkData += temp;
          temp = (char) direction + '0';
          networkData += temp;
          client.sendData(networkData);
          //cout << "(network dependent) sent move info: " << networkData << endl;
        }
      }
    }

    // still warp back even if block is moving, to prevent changing view
    if ((mx != 0 || my != 0) && warpBack) glutWarpPointer(windowCentreX, windowCentreY);
  }else // middle button
    if (mouseLookEnabled) mouseLook(x, y);

  // for an alternative manipulation method
  //
  // this is done out here (not in the above 'if')
  // so that the first time it enters the 'if' it is correct
  //mouseLastX = x;
  //mouseLastY = y;
}

// Affect all blocks with gravity
//   value - the paramater required by glutTimerFunc(). Not used.
void gravity(int value)
{
  int grounded = 0; // used for output

  for (int i = 0; i < (int) blocks.size(); i++) {
    if (!blocks[i].getMoving() && !blocks[i].getTurning() && !blocks[i].getGrounded()) {
      if (!pauseGame) blocks[i].setTargetPosition(blocks[i].getX(), blocks[i].getY()-5, blocks[i].getZ());
    }else{
      if (blocks[i].getGrounded()) grounded++;
    }
  }

  if (pauseGame) cout << "number grounded: " << grounded << endl;
  
  if (!pauseGame) {
    if (gravityTimer > GRAVITY_COUNT_MIN) gravityTimer -= GRAVITY_COUNT_DECREASE;
    // in the slow game, increase new block timer
    //if (gameNumber < GAMENUMBER_FASTGAME && newBlockTimer > NEW_BLOCK_COUNT_MIN) newBlockTimer -= NEW_BLOCK_COUNT_DECREASE;
  }

  if (!stopGravity) glutTimerFunc(gravityTimer, gravity, 0);
  else stopGravity = false;
}

// checks if there is a complete plane of blocks
void checkForPlane()
{
  vector <vector <float> > layer;
  vector <vector <float> > temp;
  vector <Block> outputBlocks;

  // go through layers of blocks looking for a complete plane
  for (int layerY = 0; layerY <= (int) BLOCK_START_Y; layerY += 5) {
    layer.clear();

    for (int i = 0; i < (int) blocks.size(); i++) {
      temp.clear();
      temp = blocks[i].getLayer(layerY);

      for (int j = 0; j < (int) temp.size(); j++) 
        layer.push_back(temp[j]);
    }

    // have we got a complete plane?
    if ((int) layer.size() == (GAMEBLOCK_WIDTH * GAMEBLOCK_DEPTH)) {
      bool found = false;
      if (network && !training) {

        for (int i = 0; i < (int) sentFoundLayer.size(); i++) {
          if (sentFoundLayer[i] == layerY) found = true;
        }
          
        // if we've not done so already, notify the other clients that a layer
        // has been found
        if (!found) {
          if (LOG_OUTPUT) cout << "sending found flag" << endl;
          sentFoundLayer.push_back(layerY);
          string networkData;
          networkData += FLAG_LAYER_FOUND;
          networkData += (char) (layerY + 1); // +1 so it can't be null
          if (network) client.sendData(networkData);
        }
    
        found = false;

        for (int i = 0; i < (int) receivedRemoveLayer.size(); i++) {
          if (receivedRemoveLayer[i] == layerY) {
            found = true;
            for (int k = i; k < (int) receivedRemoveLayer.size() - 1; k++) receivedRemoveLayer[k] = receivedRemoveLayer[k+1];
            receivedRemoveLayer.pop_back(); // remove the layer from list now it's been found and will be deleted
            for (int k = 0; k < (int) sentFoundLayer.size(); k++) {
              if (sentFoundLayer[k] == layerY) { // remove it from list
                for (int l = k; l < (int) sentFoundLayer.size() - 1; l++) sentFoundLayer[l] = sentFoundLayer[l+1];
                sentFoundLayer.pop_back();
              }
            }
            break;
          }
        }
      }

      if (!network || training || found) {
        numberOfLayers++;
        float seconds = (glutGet(GLUT_ELAPSED_TIME) - gameStartTime) / 1000.0; // no. of seconds passed since start of game
        if (LOG_OUTPUT) cout << "layer removed at time: " << seconds << " since start of this game." << endl;
        outputBlocks.clear();

        // go through all blocks, removing layers that are part of the complete plane
        for (int i = 0; i < (int) blocks.size(); i++) {
          blocks[i].unGrounded();
          if (blocks[i].removeLayer(blocks, outputBlocks, collisionArray, boundaries, blockId, layerY)) { // if a layer has been removed
            if (selectedBlock == i) selectedBlock = -1; // no longer selected because it's in pieces
            //if (removeBlock(i)) i--; // an element has been removed
            // current block is not output because it has been split
          }else{
            // the current block is ok to continue with
            outputBlocks.push_back(blocks[i]);
          }
        }

        blocks = outputBlocks;
        explosions.push_back(Explosion(layerY));
        score += 10; // 10 points
        if (scoreCount > 0) { // if got another layer too (within 50 loops)
          score += 10; // an extra 10
        }
        scoreCount = 50; // reset scoreCount
      }
    }

  } // end for layerY

  if (scoreCount > 0) scoreCount--;
}

// Create a new block
//   type - the type of block to create
void newBlock(int type)
{
  //if (!pauseGame) {
  //int type = rand() % 8;
  //if (MESS_AROUND) {
  //  if (forceCornerBlock) type = 1;
  //  else type = 0;
  //}

  // when we start a network game, we need to cancel the timer call to this
  // function and replace it with standard calls
  // the condition below will not start anymore calls once network is set,
  // and the following condition will allow us to cancel a block
  if (!cancelBlock && !gameOver) {
    blocks.push_back(Block(blockId++, type, 0, BLOCK_START_Y, 0, true, collisionArray, boundaries));
    blockScore++;
  }else cancelBlock = false;

  if (!network) {
    // decrease new block timer
    if (newBlockTimer > NEW_BLOCK_COUNT_MIN) newBlockTimer -= NEW_BLOCK_COUNT_DECREASE;
    glutTimerFunc(newBlockTimer, newBlock, rand()%8);
  }
}

// Start a new game
void newGame()
{
  gameOver = false;
  selectedBlock = -1;
  textY = TEXT_Y_START;
  gameOverCount = GAMEOVER_COUNT_MAX;

  // before we clear the blocks, find out how many cubes there are
  int numberOfCubesLeft = 0;
  for (int i = 0; i < (int) blocks.size(); i++) {
    numberOfCubesLeft += blocks[i].getNumberOfCubes();
  }
  
  blocks.clear();
  clearCollisionArray();

  // output stats
  if (LOG_OUTPUT) {
    cout << "Final score for game number " << gameNumber << ": " << score << endl;
    cout << "Block score: " << blockScore << endl;
    cout << "Number of layers: " << numberOfLayers << endl;
    cout << "Time that game lasted: " << ((glutGet(GLUT_ELAPSED_TIME) - gameStartTime) / 1000.0) << " seconds" << endl;
    cout << "Distance moved by player (master == " << client.getMaster() << "): " << player.getDistanceMoved() << endl;
    cout << "Number of cubes left: " << numberOfCubesLeft << endl;
    cout << "Gravity timer reached: " << gravityTimer << endl;
    cout << "New block timer: " << newBlockTimer << endl;
  }

  gameNumber++;
  if (LOG_OUTPUT) cout << endl << "Beginning game " << gameNumber << endl;

  score = 0, blockScore = 0;
  gameStartTime = glutGet(GLUT_ELAPSED_TIME);
  numberOfLayers = 0;
  player.resetDistanceMoved();

  if (gameNumber < GAMENUMBER_FASTGAME) {
    // slow game
    gravityTimer = (int) (GRAVITY_COUNT_START * 1.25);
    newBlockTimer = (int) (NEW_BLOCK_COUNT_START * 1.35);
  }else{
    // fast/normal game
    gravityTimer = GRAVITY_COUNT_START;
    newBlockTimer = NEW_BLOCK_COUNT_START;
  }

  // start things for the first time
  if (gotIpAddress && gameNumber == GAMENUMBER_NETWORK) {
    network = true; // the newBlock timer will no longer be called
    // cancel the next block that would come by a newBlock call
    // but only if we've been playing a game already (i.e. numberOfCubesLeft > 0)
    if (numberOfCubesLeft > 0) cancelBlock = true; 

    message = "Collaborative game!\n\nGame will start when both players are ready...";
    stopGravity = true;
    client.init();
    setReadyToSend(1);
  }else{
    if (gameNumber < GAMENUMBER_NETWORK) {
      // make a new block
      message = "Let the game begin...\n\nSolo game, ";
      if (gameNumber < GAMENUMBER_FASTGAME) message += "nice and slow to start with.";
      else message += "normal speed.";
      if (gameNumber == GAMENUMBER_SOLO) newBlock(rand()%8); // this function starts the timer for itself
      //newBlock(rand()%8); // TODO temporary!
    }else{
      message = "Starting next game... collaborative!";
    }
  }

  if (gameNumber >= GAMENUMBER_END) {
    network = false;
    message = "That's it. It's all over!\n\nPress Esc to quit!";
  }
  
  messageCount = MESSAGE_COUNT_MAX;
  
}

// End the game
void doGameOver()
{
  gameOver = true;
  string networkData;
  networkData += FLAG_GAMEOVER;
  if (network) client.sendData(networkData);
}

// Update the display
//   value - required by glutTimerFunc(). Not used.
void postRedisplay(int value)
{
  glutPostRedisplay();

  glutTimerFunc(TIMER_REDISPLAY, postRedisplay, 0);
}

// Called regularly based on a time interval.
// Contains operations that need to be performed each frame.
void idle(int value)
{
  //
  // lastFrameTime and elapsedTime used to implement pawn movement and
  // block rotation that's independent of frame rate
  //
  static int lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay();

  int newBlockType = -1;
  bool startGravity = false, receivedLock = false;

  elapsedTime=glutGet(GLUT_ELAPSED_TIME);
  //
  // Time in secs since last frame
  //
  float timeSecs = ((float) (elapsedTime - lastFrameTime)) * 0.001;

  if (mouseMoveX != 0 || mouseMoveY != 0) {
    player.turn(mouseSensitivity * mouseMoveX, mouseSensitivity * mouseMoveY);
    mouseMoveX = 0, mouseMoveY = 0;
  }
  if (pressTurnRight) player.turnRight();
  if (pressTurnLeft) player.turnLeft();
  if (pressTurnUp) player.turn(0, 0.7);
  if (pressTurnDown) player.turn(0, -0.7);
  if (pressUp) player.accelerate( timeSecs );
  if (pressDown) player.brake( timeSecs );
  if (pressRight) player.strafeRight( timeSecs );
  if (pressLeft) player.strafeLeft( timeSecs );
  if (pressRaise) player.changeGround(0.2, timeSecs);
  if (pressLower) player.changeGround(-0.2, timeSecs);

  player.move( timeSecs );

  for (int i = 0; i < (int) blocks.size(); i++) {
    blocks[i].move(blocks, boundaries, collisionArray, timeSecs);
    // check game over
    if (blocks[i].getGameOver() && !gameOver) doGameOver();
    if (gameOver) blocks[i].setInteractive(false);
  }

  for (int i = 0; i < (int) explosions.size(); i++) {
    explosions[i].go();
  }

  if (explosions.size() > 0) {
    if (explosions[0].getDead()) explosions.pop_front(); // remove dead explosion
  }
  
  if (speedCount == 0) {
    checkForPlane();
    speedCount = 6;
  }
  speedCount--;
  
  if (network && !training) {
    client.doClient(player, humans, blocks, newBlockType, startGravity, receivedLock, receivedRemoveLayer);
    if (receivedLock) drawConnectionCount = DRAW_CONNECTION_COUNT_MAX;
    if (newBlockType > -1) newBlock(newBlockType);
    if (startGravity) {
      if (LOG_OUTPUT) cout << "starting gravity" << endl;
      glutTimerFunc(gravityTimer, gravity, 0);
    }
    if (!client.getMaster()) {
      me.setTexFace(2);
      for (int i = 0; i < (int) blocks.size(); i++) {
        if (blocks[i].getMoving() || blocks[i].getTurning()) {
          blocks[i].setGotConfirmed(false);
          //cout << "moving/turning" << endl;
        }
        if (blocks[i].getGotConfirmed()) blocks[i].toConfirmed(boundaries, collisionArray);
      }
    }
  }

  // do welcome message stuff
  if (welcomeCount > 0) {
    welcomeCount--;
    textAngleY -= 0.8;
    if (welcomeCount < 200) textY -= 0.2;
    if (welcomeCount == 0) textY = TEXT_Y_START;
  }
  // do gameOver count
  if (gameOver && gameOverCount > 0) {
    gameOverCount--;
    textAngleY -= 0.8;
    if (gameOverCount < 200) textY -= 0.2;
    if (gameOverCount == 0) newGame();
  }

  // display message and frames per second
  frame++;
	elapsedTime=glutGet(GLUT_ELAPSED_TIME);
  if (training == 4 && elapsedTime - trainTimebase > TRAINING_TIME) {
    training--;
    message = "Training level 2 of 4.\nMove down using 'C'.\nMove up using the spacebar.\n\n";
    message += "You can also move up using the 'Home' key,\nand down using the 'End' key.\n";
    message += "This is helpful if using the arrow keys to move.\n\n(Press Return for next level)";
    trainTimebase = elapsedTime;
  }
  if (training == 3 && elapsedTime - trainTimebase > TRAINING_TIME) {
    training--;
    message = "Training level 3 of 4.\nPut your cross-hair over the block.\n\n";
    message += "Rotate the block with the left mouse button.\nTranslate the block with the right mouse button.\n";
    message += "\nThe block cannot be moved or rotated outside the\nboundaries.\n\n";
    message += "You cannot move blocks up or down.\nGravity will move blocks down when the game begins.\n\n";
    message += "(Press Return for next level)";
    trainTimebase = elapsedTime;
  }
  if (training == 2 && elapsedTime - trainTimebase > TRAINING_TIME) {
    training--;
    message = "Training level 4 of 4.\nMove the single block into the gap to\ncomplete the layer.\n\nComplete layers to score points.\n\n";
    message += "Bonus points are awarded for completing\nmore than one layer at a time.\n\n";
    message += "Complete this layer or press\nReturn to begin the game.";
    blocks.clear();
    selectedBlock = -1;
    clearCollisionArray();
    for (int z = -10; z < 15; z += 5) {
      for (int x = -10; x < 15; x += 5) {
        if (x != 5 || z != 0)
          blocks.push_back(Block(blockId++, 0, x, 0, z, false, collisionArray, boundaries));
      }
    }
    blocks.push_back(Block(blockId++, 0, 0, BLOCK_START_Y, 0, true, collisionArray, boundaries));
    glutTimerFunc(gravityTimer, gravity, 0);
  }
  if (training == 1 && blocks.size() == 0) {
    training = 0; // play game
    score = 0;
    newGame();
  }
  trainTimebase = glutGet(GLUT_ELAPSED_TIME); // disable training timer
	if (elapsedTime - timebase > 1000) {
		fps = frame*1000.0/(elapsedTime-timebase);
    //fpsStore.push_back((int) fps);
		timebase = elapsedTime;		
		frame = 0;
	}
  //
  // Store time for next frame
  //
  lastFrameTime = elapsedTime;

  glutTimerFunc(TIMER_IDLE, idle, 0);
}

// The main function
//   argc - the number of command line arguments
//   argv - the command line arguments
//
// Returns:
//   0 as required by standards
int main(int argc, char** argv)
{
  // initialise with command line args
  // this deals with and removes any arguments intended for GLUT
  glutInit(&argc, argv);

  if (argc > 1) {
    client.setHost(argv[1]);
    gotIpAddress = true;
    cout << "Will be using ip address: " << argv[1] << " for collaborative games." << endl;
  }else{
    gotIpAddress = false;
    cout << "Standalone." << endl;
  }

  // enable depth for 3D drawing
  // note that GLUT_STENCIL is not used because it causes slow down
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  if (!FULL_SCREEN) {
    glutInitWindowPosition (0, 0);
    //glutInitWindowSize (1280, 1024);
    glutInitWindowSize (3840, 2400);
    //glutInitWindowSize (640, 480);
    //glutInitWindowPosition (1, 450);

    // create the window
    window = glutCreateWindow ("CVE");
  }else{
    //glutGameModeString( "640x480:16@60" ); // fast!
    if (HIGH_RES) glutGameModeString( "1920x1200:24@60" ); // colossus
    else glutGameModeString( "1024x768:24@60" ); // normal

    // start fullscreen game mode
    window = glutEnterGameMode();
  }
  
  // initialise everything
  init ();

  // setup openGL functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutSpecialFunc(specialKeyPress);
  glutSpecialUpFunc(specialKeyUp);

  glutMouseFunc(mousePress);
  glutPassiveMotionFunc(mousePassiveMove);
  glutMotionFunc(mouseActiveMove);
  
  //glutIdleFunc(idle);
  
  // start the main loop
  glutMainLoop();

  return 0;
}


