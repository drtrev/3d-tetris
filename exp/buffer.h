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
 * buffer.h
 *
 * An object to buffer data and allow read and write operations
 *
 * Trevor Dodds, 2005
 */

#ifndef _BUFFER_
#define _BUFFER_

#include <iostream>

#define BUFFER_SIZE 500

using namespace std;

class Buffer {
  private:
    char buffer[BUFFER_SIZE];
    int writeCursor, readCursor, dataOnBuffer;

  public:
    Buffer();
    void write(const char*);
    int read(char[], int);
    bool getChar(char&, int);

    int getDataOnBuffer() const;
    
};

#endif
