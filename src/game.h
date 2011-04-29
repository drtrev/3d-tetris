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

#ifndef _GAME_
#define _GAME_

#include <iostream>
//
// Time parameters used by server and client (all in ms)
//   NEW_BLOCK_COUNT_START    Time before 2nd block created
//   NEW_BLOCK_COUNT_MIN      Min amount of time between block creations
//   NEW_BLOCK_COUNT_DECREASE Amount time decreases between each block
//                            0 (constant speed) or 200 (used in Trevor's
//                            evaluation)
//
#define NEW_BLOCK_COUNT_START 12000
#define NEW_BLOCK_COUNT_MIN 5000
#define NEW_BLOCK_COUNT_DECREASE 200
//#define NEW_BLOCK_COUNT_DECREASE 0

#endif
