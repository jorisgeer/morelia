// math.c - math utilities

#if 1

/* math utilities like statistics, geo, random
 */

//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <math.h>

#include "base.h"
#include "math.h"

//static ub4 msgfile;
//#include "msg.h"

//#include "mem.h"
//#include "util.h"
//#include "time.h"

#if 0
int minmax(ub4 *x, ub4 n, struct range *rp)
{
  ub4 lo = hi32;
  ub4 hi = 0;
  ub4 lopos = 0;
  ub4 hipos = 0;
  ub4 i,v;

  for (i = 0; i < n; i++) {
    v = x[i];
    if (v < lo) { lo = v; lopos = i; }
    if (v > hi) { hi = v; hipos = i; }
  }
  rp->hi = hi; rp->lo = lo;
  rp->hipos = hipos; rp->lopos = lopos;
  rp->hilo = hi - lo;
  return (n == 0);
}

int minmax2(ub2 *x, ub4 n, struct range *rp)
{
  ub4 lo = hi32;
  ub4 hi = 0;
  ub4 lopos = 0;
  ub4 hipos = 0;
  ub4 i,v;

  for (i = 0; i < n; i++) {
    v = x[i];
    if (v < lo) { lo = v; lopos = i; }
    if (v > hi) { hi = v; hipos = i; }
  }
  rp->hi = hi; rp->lo = lo;
  rp->hipos = hipos; rp->lopos = lopos;
  rp->hilo = hi - lo;
  return (n == 0);
}
#endif

// wikipedia xorshift
static ub8 xorshift64star(void)
{
  static ub8 x = 0x05a3ae52de3bbf0aULL;

  x ^= x >> 12; // a
  x ^= x << 25; // b
  x ^= x >> 27; // c
  return (x * 2685821657736338717ULL);
}

static ub8 rndstate[ 16 ];

static ub4 xorshift1024star(void)
{
  static int p;

  ub8 s0 = rndstate[p];
  ub8 s1 = rndstate[p = ( p + 1 ) & 15];
  s1 ^= s1 << 31; // a
  s1 ^= s1 >> 11; // b
  s0 ^= s0 >> 30; // c
  rndstate[p] = s0 ^ s1;

  return (ub4)(rndstate[p] * 1181783497276652981ULL);
}

// static ub4 rndmask(ub4 mask) { return (ub4)xorshift1024star() & mask; }

ub4 rnd(ub4 range)
{
  ub4 r;
  if (range == 0) r = 1;
  else r = (ub4)xorshift1024star();
  if (range && range != hi32) r %= range;
  return r;
}

double frnd(ub4 range)
{
  double x;

  x = rnd(range);
  return x;
}

int inimath(void)
{
  ub8 x;

  for (x = 0; x < 16; x++) rndstate[x] = xorshift64star();

  return 0;
}
#endif
