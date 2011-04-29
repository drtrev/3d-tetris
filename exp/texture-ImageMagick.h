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
#include <Magick++.h>


using namespace std;
using namespace Magick;

class Texture {

  private:
    static vector <string> filenames;

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
        filenames.push_back(tex[i]);
    }

    // Load an image from bitmap file
    // This is taken from the linux code of the texture mapping tutorial
    // submitted to NeHe (http://nehe.gamedev.net) by Richard Campbell
    static int imageLoad(const char *filename, Image &image)
	    //
	    // Loads a bitmap texture from a file.
	    //
	    // IN:
	    //    filename    Name of file to load texture from
	    //    image       A structure to store the image data
	    // OUT:
	    //    image       Filled with image data
	    // Returns:
	    //    true if succesful, fals otherwise.
	    //
    {
	    try {
 cerr << ':' << filename << ':' << endl;
		    image.read(filename);
		    image.flip();

		    image.getConstPixels(0,0,image.columns(),image.rows());
		    return true;
	    }
	    catch (Exception &error_)
	    {
		    return false;
	    }

    }


    // initialise, load and store the textures
    static void loadTextures()
    {
      Image tempTex;                   // Temporary image structure for image data

      GLuint textures[(int) filenames.size()];

      glGenTextures(filenames.size(), &textures[0]); // Generate so many textures

      // Load all textures
      for(unsigned int x = 0; x < filenames.size(); x++)
      {
        // Allocate memory space for texture
        //tempTex = (Image *) malloc(sizeof(Image)); // Allocate memory for textures
        if(!imageLoad(filenames[x].c_str(), tempTex))
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
	unsigned char *data = new unsigned char[tempTex.columns() * 
                                                tempTex.rows() * 3];
	tempTex.writePixels(RGBQuantum, data);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tempTex.columns(), tempTex.rows(),
                          GL_RGB, GL_UNSIGNED_BYTE, data);
	delete [] data;

        texId.push_back(textures[x]);
      }

    }

};

#endif
