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
 * texture.h
 *
 * A class to manage textures, with texture loading code from Nehe
 * (http://nehe.gamedev.net) submitted by Richard Campbell
 *
 * Trevor Dodds, 2005
 */

#ifndef _TEXTURE_
#define _TEXTURE_

#include <iostream>
#include <vector>
#include <GL/glut.h>

using namespace std;

/* Changes to make it work on SGI machines:
 * The problem was four bytes were allocated to a long in imageLoad(). This is now an unsigned int instead.
 * Also the malloc's/free's have been updated to use C++'s new/delete
 */

// NOTE: no longer needed/used
// taken from JH/Ben Hammett's 3D drawing project
// byte swapping for SGI machines
#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))

void ByteSwap(unsigned char * b, int n)
//
// When loading the textures in the level 6 lab, there were issues with loading
// the texture files. Level 6 machines read data as big-Endian, whereas normal
// SoC machines read data as little-Endian. This function swaps between the two.// see http://www.codeproject.com/cpp/endianness.asp for a better explanation.
//
// IN:
//     *char
//     *n
// OUT:
//     none.
// Returns:
//     none.
//
{
   int i = 0;
   int j = n-1;
   while (i < j){
      char tmp = b[i];
      b[i] = b[j];
      b[j] = tmp;
      i++;
      j--;
   }
}


class Texture {

  private:
    static vector <char*> filenames;
    struct Image {
      unsigned sizeX;
      unsigned sizeY;
      char *data;
    };

  public:
    static vector <int> texId; // the texture numbers

    static void init()
    {
      char* texPathPtr = getenv("TEXTURE_PATH");
      string texPath = "";
      if (texPathPtr != NULL) {
        texPath = texPathPtr;
        texPath += "/";
      }

      string tex[9];
      tex[0] = texPath + "welcome.bmp";
      tex[1] = texPath + "gameOver.bmp";
      tex[2] = texPath + "monkeyFace.bmp";
      tex[3] = texPath + "catFace.bmp";
      tex[4] = texPath + "concrete14.bmp";
      tex[5] = texPath + "marble.bmp";
      tex[6] = texPath + "explosion.bmp";
      tex[7] = texPath + "clouds2.bmp";
      tex[8] = texPath + "mountain.bmp";

      for (int i = 0; i < 9; i++)
        filenames.push_back((char*) tex[i].c_str());
    }

    // Load an image from bitmap file
    // This is taken from the linux code of the texture mapping tutorial
    // submitted to NeHe (http://nehe.gamedev.net) by Richard Campbell
    static int imageLoad(char *filename, Image *image)
    {
      FILE *file;
      unsigned size;                 // size of the image in bytes.
      unsigned i;                    // standard counter.
      unsigned short int planes;          // number of planes in image (must be 1)
      unsigned short int bpp;             // number of bits per pixel (must be 24)
      char temp;                          // temporary color storage for bgr-rgb conversion.

      // make sure the file is there.
      if ((file = fopen(filename, "rb"))==NULL)
      {
        cerr << "Texture file not found: " << filename << endl;
        return 0;
      }

      // seek through the bmp header, up to the width/height:
      fseek(file, 18, SEEK_CUR);

      // read the width
      if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
        printf("Error reading width from %s.\n", filename);
        return 0;
      }
      printf("Width of %s: %u\n", filename, image->sizeX);
      //ByteSwap5(image->sizeX);
      
      //printf("Width of %s: %u\n", filename, image->sizeX);

      // read the height
      if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
        printf("Error reading height from %s.\n", filename);
        return 0;
      }
      //ByteSwap5(image->sizeY);
      printf("Height of %s: %u\n", filename, image->sizeY);

      // calculate the size (assuming 24 bits or 3 bytes per pixel).
      size = image->sizeX * image->sizeY * 3;

      // read the planes
      if ((fread(&planes, 2, 1, file)) != 1) {
        printf("Error reading planes from %s.\n", filename);
        return 0;
      }
      if (planes != 1) {
        printf("Planes from %s is not 1: %u\n", filename, planes);
        return 0;
      }

      // read the bpp
      if ((i = fread(&bpp, 2, 1, file)) != 1) {
        printf("Error reading bpp from %s.\n", filename);
        return 0;
      }
      if (bpp != 24) {
        printf("Bpp from %s is not 24: %u\n", filename, bpp);
        return 0;
      }

      // seek past the rest of the bitmap header.
      fseek(file, 24, SEEK_CUR);

      // read the data.
      cout << "size: " << size << endl;
      image->data = new char[size];
      if (image->data == NULL) {
        printf("Error allocating memory for color-corrected image data");
        return 0;
      }

      if ((i = fread(image->data, size, 1, file)) != 1) {
        printf("Error reading image data from %s.\n", filename);
        return 0;
      }

      for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
        temp = image->data[i];
        image->data[i] = image->data[i+2];
        image->data[i+2] = temp;
      }

      // we're done.
      return 1;
    }

    // initialise, load and store the textures
    static void loadTextures()
    {
      Image *tempTex;                   // Temporary image structure for image data

      GLuint textures[(int) filenames.size()];

      glGenTextures(filenames.size(), &textures[0]); // Generate so many textures

      // Load all textures
      for(unsigned int x = 0; x < filenames.size(); x++)
      {
        // Allocate memory space for texture
//        tempTex = (Image *) malloc(sizeof(Image)+256*256*3); // Allocate memory for textures
        tempTex = new Image();

        if(!imageLoad(filenames[x], tempTex))
        {
          cerr << "Error allocating space for texture" << endl;
          cerr << "Texture: " << x << " -- " << filenames[x] << endl;
          exit(0);
        }

        glBindTexture(GL_TEXTURE_2D, textures[x]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // mipmap!
        // GL_LINEAR_MIPMAP_NEAREST chooses mipmap based on average of four surrounding texels
        // GL_LINEAR_MIPMAP_LINEAR perhaps looks a little bit better but slows it down
        // default mipmap is GL_NEAREST_MIPMAP_LINEAR, but this leaves a low res strip in the middle

        // gluBuildMipmaps calls several glTexImage2D's with the smaller images it has created
        //glTexImage2D(GL_TEXTURE_2D, 0, 3, tempTex->sizeX, tempTex->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTex->data);

        //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, tempTex->data);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tempTex->sizeX, tempTex->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tempTex->data);

        texId.push_back(textures[x]);
        delete [] tempTex->data;
        delete tempTex;
      }

    }

};

#endif
