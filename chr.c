/* chr.c - character utilities

   This file is part of Morelia, a subset of Python with emphasis on efficiency.

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
#include "chr.h"

static const ub1 hextab[16] = "0123456789abcdef";
static const ub1 esctab[] = "abtnvfr";

// for a in {A..Z}; do echo "  ['$a'] = Calpha;"; done

const enum Ctype Ctab[256] = {
  ['A'] = Calpha,
  ['B'] = Calpha,
  ['C'] = Calpha,
  ['D'] = Calpha,
  ['E'] = Calpha,
  ['F'] = Calpha,
  ['G'] = Calpha,
  ['H'] = Calpha,
  ['I'] = Calpha,
  ['J'] = Calpha,
  ['K'] = Calpha,
  ['L'] = Calpha,
  ['M'] = Calpha,
  ['N'] = Calpha,
  ['O'] = Calpha,
  ['P'] = Calpha,
  ['Q'] = Calpha,
  ['R'] = Calpha,
  ['S'] = Calpha,
  ['T'] = Calpha,
  ['U'] = Calpha,
  ['V'] = Calpha,
  ['W'] = Calpha,
  ['X'] = Calpha,
  ['Y'] = Calpha,
  ['Z'] = Calpha,

  ['a'] = Calpha,
  ['b'] = Calpha,
  ['c'] = Calpha,
  ['d'] = Calpha,
  ['e'] = Calpha,
  ['f'] = Calpha,
  ['g'] = Calpha,
  ['h'] = Calpha,
  ['i'] = Calpha,
  ['j'] = Calpha,
  ['k'] = Calpha,
  ['l'] = Calpha,
  ['m'] = Calpha,
  ['n'] = Calpha,
  ['o'] = Calpha,
  ['p'] = Calpha,
  ['q'] = Calpha,
  ['r'] = Calpha,
  ['s'] = Calpha,
  ['t'] = Calpha,
  ['u'] = Calpha,
  ['v'] = Calpha,
  ['w'] = Calpha,
  ['x'] = Calpha,
  ['y'] = Calpha,
  ['z'] = Calpha,

  ['_'] = Calpha,

  ['0'] = Cnum,
  ['1'] = Cnum,
  ['2'] = Cnum,
  ['3'] = Cnum,
  ['4'] = Cnum,
  ['5'] = Cnum,
  ['6'] = Cnum,
  ['7'] = Cnum,
  ['8'] = Cnum,
  ['9'] = Cnum,

  [' '] = Cws,
  ['\t'] = Cws,
  ['\n'] = Cnl,
  ['\\'] = Cbs,
  ['\''] = Csq,
  ['"'] = Cdq,
  ['.'] = Cdot,
  [','] = Ccomma,
  ['#'] = Chsh,
  ['+'] = Cpls,
  ['-'] = Cmin,
  ['*'] = Cast,
  ['?'] = Cqst,
  ['|'] = Cor,
  ['='] = Ceq,
  ['`'] = Cbtk,

  ['{'] = Cother,
  ['}'] = Cother,
  ['['] = Cother,
  [']'] = Cother,

  ['('] = Crdo,
  [')'] = Crdc,

  ['&'] = Cother,
  ['%'] = Cother,
  ['^'] = Cother,
  ['~'] = Cother,
  ['!'] = Cother,
  ['@'] = Cother,
  ['$'] = Cother,
  ['/'] = Cother,

  ['<'] = Cother,
  ['>'] = Cother,

  [':'] = Cother,
  [';'] = Cother,
};

static ub2 dochprint(ub1 c,ub1 *p)
{
  if (Ctab[c] == 0) {
    p[0] = '0';
    p[1] = 'x';
    p[2] = hextab[c >> 4];
    p[3] = hextab[c & 0xf];
    return 4;
  } else if (c < 0x0e) {
    c = esctab[c - 7];
    p[0] = '\\'; p[1] = c;
    return 2;
  } else if (c == '\\') {
    p[0] = '\\'; p[1] = c;
    return 2;
  } else {
    *p = c;
    return 1;
  }
}

cchar *chprint(ub1 c)
{
  static ub1 buf[4 * 8];
  ub1 *p;
  ub2 n;
  static ub2 bufno;

  bufno = (bufno + 1) & 3;
  p = buf + bufno * 8;

  n = dochprint(c,p);
  p[n] = 0;
  return (cchar *)p;
}

const ub1 *chprints(const ub1 *s,ub2 n)
{
  ub4 pos=0,len = min(252,n);
  ub1 c;
  const ub1 *q;
  static ub1 buf[256];

  while ( (c=*s++) && pos < len ) {
    if (Ctab[c] && c >= ' ') buf[pos++] = c;
    else pos += dochprint(c,buf+pos);
  }
  buf[pos] = 0;
  return buf;
}

ub2 underline(char *buf,ub2 slen)
{
  char tmp[1024];
  ub2 n=0;
  ub2 len=0;

  while (len < 1020 && n < slen && buf[n]) {
    tmp[len]   = (char)0xcc;
    tmp[len+1] = (char)0xb2;
    tmp[len+2] = buf[n++];
    len += 3;
  }
  memcpy(buf,tmp,len);
  buf[len] = 0;
  return len;
}

