/* mem.c - memory allocation wrappers and provisions

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

/* Memory allocation:
   wrappers, pool allocators
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "base.h"

#include "os.h"

static ub4 msgfile = Shsrc_mem;
#include "mem.h"

#include "msg.h"

#undef fatal
#define fatal(fln,fmt,...) fatalfln(FLN,fln,hi32,fmt,__VA_ARGS__)

static const ub4 Maxmem_mb = 4096;

static const ub4 mmap_thres = 65536;

static const ub4 mini_thres = 1024;

static const ub4 maxalign = 16;
static const ub4 stdalign = 8;

static ub4 pagemask;

static ub2 aihshbit  = 9;
static ub4 aihshlen  = 512;
static ub4 aihshmask = 511;
static ub4 aiuse;

static ub4 totalkb,maxkb;

static cchar *descs[Memdesc * Shsrc_count];
static ub4 flns[Memdesc * Shsrc_count];
static ub2 elsizes[Memdesc * Shsrc_count];

struct ainfo {
  void *ptr;
  // ub4 nelem;
  ub2 allocanchor;
  ub2 freeanchor;
};

static struct ainfo *aitab;

static void addsum(ub4 b)
{
  ub4 kb = b >> 10;

  totalkb += kb;
  if (totalkb > maxkb) maxkb = totalkb;
}

static void subsum(ub4 b)
{
  ub4 kb = b >> 10;

  totalkb -= min(kb,totalkb);
}

// Thomas Wang 64 bit integer hash
// http://web.archive.org/web/20120720045250/http://www.cris.com/~Ttwang/tech/inthash.htm
#ifdef __clang
__attribute__((no_sanitize("unsigned-integer-overflow")) )
#endif
static ub8 ptrhash(ub8 key)
{
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}

static ub4 align4(ub4 x,ub4 a)
{
  ub4 r;

  if (a == 0) ice(FLN,hi32,"zero align for %u",x);
  r = x & ~(a-1);

  return (r == x ? r : r + a);
}

// mini alloc for small, nonfreeable blocks
#define Minchk 65536
#define Minmax (1U << 24)

static void *minpool;
static ub4 minpos,mintop,mintot;

void *minalloc_fln(ub4 fln,ub4 n,ub2 align,ub2 fill)
{
  ub1 *p;
  ub4 inc;

  vrbfln(fln,"mem.%u minalloc %u",__LINE__,n);

  if (align == 0) {
    vrbfln(fln,"mem.%u minalloc %u align 0 = 16",__LINE__,n);
    align = stdalign;
  }

  if (n >= Minchk || n + align >= Minchk) fatal(fln,"not mini %u",n);
  else if (mintot + n > Minmax) fatal(fln,"mini pool exceeds %u MB",Minmax >> 20);

  minpos = align4(minpos+1,align);
  if (minpool == nil) inc = Minchk;
  else if (minpos + n >= mintop) {
    inc = (n < Minchk ? Minchk : Minchk * 2);
  } else inc = 0;
  if (inc) {
    vrb("mmap %u",inc);
    minpool = osmmap(inc,ub1);
    minpos = 2 * maxalign;
    mintop = inc;
    mintot += inc;
    addsum(inc);
  }
  p = (ub1 *)minpool + minpos;
  if (fill <= 0xff) memset(p,fill,n);
  minpos += n;
  return p;
}

// medium alloc for nonfreeable blocks
static ub4 medchk = 0x8000;
static void *medpool;
static ub4 medpos;

void *medalloc_fln(ub4 fln,ub4 n,ub2 align)
{
  ub1 *p;
  ub4 nn;
  ub1 bit;

  if (n >= (1U << 26)) fatal(fln,"medalloc %u`B",n);

  medpos = align4(medpos+1,align);
  if (medpool == nil || medpos + n >= medchk) {
    if (medchk <= (1U << 20)) medchk++;
    nn = max(medchk,n);
    if (nn >= (1U << 26)) fatal(fln,"medalloc %u`B",nn);
    medchk = nxpwr2(nn,&bit);
    if (medchk > (1U << 22)) warnfln(fln,"mem.%u medalloc chunk %u`B",__LINE__,medchk);
    medpool = osmmap(medchk,ub1);
    medpos = 2 * maxalign;
    addsum(medchk);
  }
  p = (ub1 *)medpool + medpos;
  medpos += n;
  return p;
}

static struct ainfo *getai(const void *p)
{
  ub8 hsh8 = ptrhash((ub8)p);
  ub4 hsh2,hsh = hsh8 & aihshmask;
  ub4 pos,pos0;
  struct ainfo *ai;

  ai = aitab + hsh;
  if (ai->ptr == p) {
    return ai;
  }

  hsh2 = (hsh8 >> aihshbit) & aihshmask;
  pos = (hsh + hsh2) & aihshmask;
  ai = aitab + pos;
  if (ai->ptr == p) return ai;
  do {
    pos0 = pos;
    pos = (pos + 1) & aihshmask;
    ai = aitab + pos;
    if (ai->ptr == p) return ai;
    else if (ai->ptr == nil) return nil;
  } while (pos != pos0);
  return nil;
}

static struct ainfo *putai(void *p)
{
  ub8 hsh8 = ptrhash((ub8)p);
  ub4 hsh2,hsh = hsh8 & aihshmask;
  ub4 pos,pos0;
  struct ainfo *ai;

  ai = aitab + hsh;
  if (ai->ptr == nil) {
    ai->ptr = p;
    aiuse++;
    return ai;
  } else if (ai->ptr == p) return ai;
  if (aiuse == aihshlen) return nil;

  hsh2 = (hsh8 >> aihshbit) & aihshmask;
  pos = (hsh + hsh2) & aihshmask;
  ai = aitab + pos;
  if (ai->ptr == nil) {
    ai->ptr = p;
    aiuse++;
    return ai;
  } else if (ai->ptr == p) return ai;
  do {
    pos0 = pos;
    pos = (pos + 1) & aihshmask;
    ai = aitab + pos;
    if (ai->ptr == nil) {
      ai->ptr = p;
      aiuse++;
      return ai;
    } else if (ai->ptr == p) return ai;
  } while (pos != pos0);
  info("adr info table size %u full: not releasing",aihshlen);
  aiuse = aihshlen;
  return nil;
}

static void iniai(void)
{
  aitab = (struct ainfo *)minalloc(aihshlen * sizeof(struct ainfo),8,0);
}

void *alloc_fln(ub4 fln,ub4 nelem,ub4 elsiz,ub2 fil,const char *desc,ub2 counter)
{
  ub8 n8,x8;
  ub4 n,nn,na,nm,totalmb,align = min(elsiz,maxalign);
  ub1 *p;
  struct ainfo *ai;
  ub2 allan,mod;

  if (desc == nil) desc = "";

  vrb("+alloc %u * %u @%u %s",nelem,elsiz,align,desc);

  // check for zero
  if (nelem == 0) fatal(fln,"zero elems for %s",desc);
  else if (nelem == hi32) fatal(fln,"4G elems for %s",desc);
  if (elsiz == 0) fatal(fln,"zero elsize for %s",desc);
  else if (elsiz == hi32) fatal(fln,"4G elsize for %s",desc);
  else if (elsiz > hi16) fatal(fln,"64KB+ elsize for %s",desc);

  n8 = (ub8)nelem * elsiz;
  if (n8 >= hi32) fatal(fln,"%u` * %u` = 4GB+ for %s",nelem,elsiz,desc);
  n = (ub4)n8;

  if (n < mini_thres) {
    p = minalloc_fln(fln,n,align,fil);
    return p;
  }

  nm = n >> 20;

  if (nm >= Maxmem_mb) {
    fatal(fln,"exceeding %u MB limit by %u %s",Maxmem_mb,nm,desc);
  }
  totalmb = totalkb >> 10;
  if (totalmb + nm >= Maxmem_mb) {
    fatal(fln,"exceeding %u MB limit by %u+%u=%u MB %s",Maxmem_mb,totalmb,nm,nm + totalmb,desc);
  }

  nn = align4(n+4,align);
  if (nn >= mmap_thres) {
    na = align4(nn,ospagesize);
    infofln(fln,"mem.%u Alloc %u`B %s",__LINE__,na,desc);
    p = osmmap(na,ub1);
    if (!p) fatal(fln,"cannot alloc %u`B, total %u MB for %s: %m",na,totalmb,desc);
    *(ub4 *)p = na;
    p += align4(4,align);
    if (fil == 0) fil = Mo_nofill;
  } else {
    nn = align4(n+1,align);
    na = nn + maxalign + align;
    vrbfln(fln,"mem.%u alloc %u`B %s",__LINE__,n,desc);
    p = malloc(na);
    if (!p) fatal(fln,"cannot alloc %u`B, total %u MB for %s: %m", n,totalmb,desc);
    x8 = (ub8)p & pagemask;
    if (x8 <= maxalign) p = (ub1 *)(~x8 + maxalign + align); // note - not freed
    else {
      *p = 0xff;
      p += align4(1,align);
    }
  }

  if (fil < Mo_nofill) memset(p,fil,n);

  addsum(na);

  ai = getai(p);
  if (ai == nil) ai = putai(p);
  if (ai == nil) return p;

  mod = fln >> 16;
  allan = mod * Memdesc + counter;
  descs[allan] = desc;
  flns[allan] = fln;
  if ( (n = elsizes[allan]) && n != elsiz) fatal(fln,"elsize %u vs %u %s",n,elsiz,desc);
  elsizes[allan] = elsiz;

  ai->allocanchor = allan;
  ai->freeanchor = hi16;

  return p;
}

void afree_fln(ub4 fln,void *p,const char *desc,ub2 counter)
{
  struct ainfo *ai;
  ub4 nelem,elsiz,ofs;
  ub4 n;
  ub8 x8;
  ub2 allan,freean,anchor,mod;
  ub1 x;

  if (desc == nil) desc = "";

  if (!p) { errorfln(fln,FLN,"free nil pointer for %s",desc); return; }

  x8 = (ub8)p & pagemask;
  if (x8 > maxalign) { // malloc or min/med
    x = *(ub1 *)(p-1);
    if (x == 0) return; // min/med
    else if (x != 0xff) fatal(fln,"free of unknown ptr type %x %p",x,p);
  }

  ai = getai(p);
  if (ai) { // minalloc or full ai
    freean = ai->freeanchor;
    allan = ai->allocanchor;
    elsiz = elsizes[allan];

    if (freean != hi16) {
      infofln(flns[freean],"mem.%u location of previous free",__LINE__);
      errorfln(fln,FLN,"double free of pointer %p '%s'",p,desc);
      infofln(flns[allan],"mem.%u location of allocation '%s'",__LINE__,descs[allan]);
      return;
    }

    mod = fln >> 16;
    anchor = mod * Memdesc + counter;
    descs[anchor] = desc;
    flns[anchor] = fln;
    elsizes[anchor] = elsiz;
    ai->freeanchor = anchor;
  }

  x8 = (ub8)p & pagemask;
  ofs = (ub4)x8;
  if (ofs <= maxalign) { // must be mmap()
    x8 = ~x8;
    p = (void *)x8;
    n = *(ub4 *)p;
    osmunmap(p,n);
    subsum(n);
  } else if (ai && ofs > 2 * maxalign) {
    free(p);
  }
}

void *allocset_fln(ub4 fln,struct mempart *parts,ub2 npart,ub2 fil,const char *desc,ub2 counter)
{
  ub4 len=0,len2;
  ub4 nel,siz;
  ub2 align,align0=1;
  ub2 part;
  char *bas;

  for (part = 0; part < npart; part++) {
    nel = parts[part].nel;
    siz = parts[part].siz;
    align = min(siz,16);
    if (part == 0) {
      align0 = align;
    } else len = align4(len,align);
    len += nel * siz;
  }

  len2 = (len + align0) / align0;
  bas = alloc_fln(fln,len2,align0,fil,desc,counter);
  len = 0;
  for (part = 0; part < npart; part++) {
    nel = parts[part].nel;
    siz = parts[part].siz;
    align = min(siz,16);
    len = align4(len,align);
    parts[part].ptr = bas+len;
    parts[part].ismmap = (len2 * align0) >= mmap_thres;
    len += nel * siz;
  }
  return bas;
}

void achkfree(void)
{
  struct ainfo *ai;
  ub4 andx,n = 0;
  ub4 y;
  ub2 allan;

  for (andx = 0; andx < aihshlen; andx++) {
    ai = aitab + andx;
    if (ai->ptr && ai->freeanchor == hi16) {
      allan = ai->allocanchor;
      n++;
      y = elsizes[allan];
      warnfln(flns[allan],"mem.%u not freed n * %u` '%s'",__LINE__,y,descs[allan]);
    }
  }
  if (n == 0) return;

  info("%u blocks not freed",n);
}

ub1 *blkexp_fln(ub4 fln,struct expmem *xp,ub4 cnt,ub4 typsiz)
{
  ub1 *bas = xp->bas;
  ub4 ocnt,ncnt;
  ub4 pos = xp->pos;
  ub4 inc = xp->inc;
  ub2 elsiz = xp->elsiz;
  ub2 inilim = 4096;
  ub2 align = max(xp->align,elsiz);

  if (typsiz != elsiz) ice(fln,hi32,"blk %s: elsiz %u vs %u",xp->desc,elsiz,typsiz);
  if (bas == nil) {
    ncnt = max(xp->ini,cnt);
    if (ncnt * elsiz < inilim) {
      bas = minalloc_fln(fln,ncnt * elsiz,align,0);
      xp->min = 1;
    } else {
      info("mem.%u blkexp %u`B",__LINE__,ncnt * elsiz);
      bas = osmmapfln(fln,ncnt,elsiz);
      addsum(ncnt * elsiz);
    }
    xp->bas = bas;
    xp->top = ncnt;
  } else if (pos + cnt >= xp->top) {
    ncnt = max(cnt,inc) + xp->top;
    ocnt = xp->top;

    if (xp->min) {
      info("mem.%u blkexp +%u`B",__LINE__,(ncnt - ocnt) * elsiz);
      bas = osmmapfln(fln,ncnt,elsiz);
      memcpy(bas,xp->bas,ocnt * elsiz);
      xp->min = 0;
    } else {
      bas = osmremapfln(fln,bas,elsiz,ocnt,ncnt);
      addsum((ncnt - ocnt) * elsiz);
    }
    xp->bas = bas;
    xp->top = ncnt;
  }
  xp->pos += cnt;
  return bas + pos * elsiz;
}

void *nearblock(void *p) // todo
{
  return p;
}

#if 0
int memrdonly(void *p,ub8 len)
{
  if ((ub8)p & 4095) return 0;
  return osmemrdonly(p,len);
}
#endif

ub4 meminfo(void) { return osmeminfo(); }

void memcfg(ub4 maxvm)
{
#ifdef Asan
  infofln(FLN,"no soft VM limit in asan");
  return;
#endif

  info("setting soft VM limit to %u GB",maxvm);
//  Maxmem_mb = (maxvm == hi24 ? hi32 : maxvm * 1024);
}

void inimem(void)
{
  iniai();

  pagemask = ospagesize - 1;
}

void eximem(bool show)
{
  vrb("max ai use %u",aiuse);
  if (show & globs.resusg) info("max net   mem use %lu`B",(ub8)maxkb << 10);
}
