/* morelia.c - main program

   This file is part of Morelia, a variation on a Python theme.

   Copyright © 2023 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Morelia is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, typically in the file License.txt
   If not, see http://www.gnu.org/licenses.
 */

// Main driver : parse comandline and roll the ball

#include <string.h>

#include "base.h"

static cchar *srcnam;
static cchar *cmdprog;

struct globs globs;

static int cmdline(int argc, char *argv[])
{
  while (argc) { // options
  }

  if (cmdprog == nil) {
    if (argc) {
      srcnam = *argv;
      argc--; argv++;
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  int rv=0;
  cchar *prgnam=nil;

  if (argc > 0 && argv[0]) {
    prgnam = strrchr(argv[0],'/');
    if (prgnam) prgnam++; else prgnam = argv[0];
    argc--; argv++;
  }
  if (prgnam == nil || *prgnam == 0) prgnam = "morelia";
  globs.prgnam = prgnam;

  rv = cmdline(argc,argv);
  if (rv) {
    return 1;
  }

  return rv;
}
