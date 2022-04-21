/* base.c - generic base utility functions

   This file is part of Morelia, a subset of of Python with emphasis on efficiency.

   Copyright © 2022 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   mpy is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program, typically in the file License.txt
   If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "base.h"

ub1 atox1(ub1 c)
{
  if (c >= '0' && c <= '9') return c - '0';
  else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  else return 16;
}

ub4 sat32(ub8 x,ub8 y)
{
  ub8 xy = x * y;
  return (ub4)min(xy,hi32);
}

ub4 nxpwr2(ub4 x,ub1 *bit)
{
  ub4 pwr2 = 1;

  if (x > 0x80000000) return hi32;

  while (pwr2 < x) pwr2 <<= 1;
  *bit = pwr2;
  return x;
}

ub2 msb(ub8 x)
{
  ub2 b = 63;
  while (b && (x & (1UL << b)) == 0) b--;
  return b;
}

ub2 cntbits(ub4 x)
{
  ub2 n = 0;
  while (x) {
    if (x & 1) n++;
    x >>= 1;
  }
  return n;
}

ub2 cntbits8(ub8 x)
{
  ub2 n = 0;
  while (x) {
    if (x & 1) n++;
    x >>= 1;
  }
  return n;
}

// increase to multiply-friendly
ub2 expndx(ub2 x,ub2 lim)
{
  ub2 ymin, y = x;
  ub2 n,nmin=64;

  ymin = y;
  while (y < x + lim) {
    n = cntbits(y);
    if (n == 1) return y;
    else if (n < nmin) { nmin = n; ymin = y; }
    y++;
  }
  return ymin;
}

int inibase(void)
{
  return 0;
}
