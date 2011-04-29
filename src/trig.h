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
 * trig.h
 *
 * An old-fashioned technique for speeding up trig calculations.
 * Values are calculated once and stored in an array.
 *
 * Trevor Dodds, 2005
 */

#ifndef _TRIG_
#define _TRIG_

#include <cmath>
#include <iostream>

using namespace std;

class Trig {

  private:
    static float cosValues[721];
    static float sinValues[721];
    static float asinValues[121];

  protected:

  public:

    static void init() {
      for (int i = 0; i < 721; i++) {
        cosValues[i] = cos((i-360)/180.0 * 3.14);
        sinValues[i] = sin((i-360)/180.0 * 3.14);
      }
      // calculate asin from -1.0 to 2.0
      for (int i = 0; i < 121; i++) {
        asinValues[i] = asin(i/40.0 - 1.0) / 3.14 * 180.0;
      }
    }

    static float sn(float n) {
      // these are called from shoot.cc with a +180 so could go over 360
      if (n > 360) n -= 360;
      if (n > 360 || n < -360) {
        cerr << "Trig::sn() - n out of range. n = " << n << endl;
        return 0;
      }
      return sinValues[(int) n+360];
    }

    static float cs(float n) {
      if (n > 360) n -= 360;
      if (n > 360 || n < -360) {
        cerr << "Trig::cs() - n out of range. n = " << n << endl;
        return 0;
      }
      return cosValues[(int) n+360];
    }

    static float asn(float n) {
      n = (n + 1) * 40;
      if ((int) n < 0 || (int) n > 120) {
        cout << "Trig::asn() - n out of range. n = " << n << " - original: " << (n/40.0-1.0) << endl;
      }
      return asinValues[(int) n];
    }
  
};

#endif
