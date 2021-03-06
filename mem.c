/* mem.c - memory allocation wrappers and provisions

   This file is part of Morelia, a subset of Python with emphasis on efficiency.

   Copyright © 2022 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Morelia is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program, typically in the file License.txt
   If not, see http://www.gnu.org/licenses.
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
static const ub4 minalignmask = 7;

#define Aihshbit1 8
#define Aihshbit2 10

static ub4 pagemask;

static ub4 aiuses[2];

static ub4 totalkb,maxkb;

static cchar *descs[Memdesc * (Shsrc_mem+1)];
static ub4 flns[Memdesc * (Shsrc_mem+1)];
static ub2 elsizes[Memdesc * (Shsrc_mem+1)];

struct ainfo {
  const void *ptr;
  ub4 len;
  ub2 allocanchor;
  ub2 freeanchor;
};

static struct ainfo aitab1[1U << Aihshbit1];
static struct ainfo aitab2[1U << Aihshbit2];

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

static ub4 align4(ub4 fln,ub4 x,ub4 a,cchar *dsc)
{
  ub4 r;

  switch (a) {
  case 0: ice(fln,hi32,"zero align for %u",x);
  case 1: return x;
  case 2: return x & 1 ? x+1 : x;
  default:
    if (a & (a-1)) ice(fln,hi32,"align %u not power of two for %s",a,dsc);
    else if (a > maxalign) ice(fln,hi32,"align %u above %u for %s",a,maxalign,dsc);
    r = x & ~(a-1);
    return (r == x ? r : r + a);
  }
}

// mini alloc for small, nonfreeable blocks
#define Minchk 65536
#define Minmax (1U << 24)

static void *minpool;
static ub4 minpos,mintop,mintot;

void *minalloc_fln(ub4 fln,ub4 n,ub2 align,ub2 fill,cchar *dsc)
{
  ub1 *p;
  ub4 inc;

  vrbfln(fln,"mem.%u minalloc %u@%u for %s",__LINE__,n,align,dsc);

  if (align == 0) {
    vrbfln(fln,"mem.%u minalloc %u align 0 -> %u %s",__LINE__,n,stdalign,dsc);
    align = stdalign;
  }

  if (n >= Minchk || n + align >= Minchk) fatal(fln,"not mini %u %s",n,dsc);
  else if (n == 0) fatal(fln,"zero len for '%s'",dsc);
  else if (mintot + n > Minmax) fatal(fln,"mini pool exceeds %u MB %s",Minmax >> 20,dsc);

  minpos = align4(fln,minpos,align,dsc);
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
  if (fill <= 0xff) { vrbo("mem.%u set %x",__LINE__,fill); memset(p,fill,n); }
  minpos += n;
  return p;
}

// medium alloc for nonfreeable blocks
static ub4 medchk = 0x8000;
static void *medpool;
static ub4 medpos;

void *medalloc_fln(ub4 fln,ub4 n,ub2 align,cchar *dsc)
{
  ub1 *p;
  ub4 nn;
  ub1 bit;

  if (n >= (1U << 26)) fatal(fln,"medalloc %u`B %s",n,dsc);
  else if (n == 0) fatal(fln,"zero len %s",dsc);

  medpos = align4(fln,medpos,align,dsc);
  if (medpool == nil || medpos + n >= medchk) {
    if (medchk <= (1U << 20)) medchk++;
    nn = max(medchk,n);
    if (nn >= (1U << 26)) fatal(fln,"medalloc %u`B %s",nn,dsc);
    medchk = nxpwr2(nn,&bit);
    if (medchk > (1U << 22)) warnfln(fln,"mem.%u medalloc chunk %u`B %s",__LINE__,medchk,dsc);
    medpool = osmmap(medchk,ub1);
    medpos = 2 * maxalign;
    addsum(medchk);
  }
  p = (ub1 *)medpool + medpos;
  medpos += n;
  return p;
}

static struct ainfo *getai(const void *p,bool ismmap,bool add)
{
  ub8 hsh8 = ptrhash((ub8)p);
  ub4 hsh2,hsh,mask,bit;
  ub4 pos,pos0;
  struct ainfo *ai,*aibas;

  if (ismmap) {
    aibas = aitab2;
    bit = Aihshbit2;
    mask = (1U << Aihshbit2)-1;
  } else {
    aibas = aitab1;
    bit = Aihshbit1;
    mask = (1U << Aihshbit1)-1;
  }

  hsh = hsh8 & mask;
  ai = aibas + hsh;
  if (ai->ptr == p) {
    return ai;
  }

  hsh2 = (hsh8 >> bit) & mask;
  pos = (hsh + hsh2) & mask;
  ai = aibas + pos;
  if (ai->ptr == p) return ai;
  do {
    pos0 = pos;
    pos = (pos + 1) & mask;
    ai = aibas + pos;
    if (ai->ptr == p) return ai;
    else if (ai->ptr == nil) {
      if (add == 0) return nil;
      ai->ptr = p;
      aiuses[ismmap]++;
      return ai;
    }
  } while (pos != pos0);

  if (add) info("adr info table %u size %u full: not releasing",ismmap,mask+1);
  return nil;
}

static ub4 chkfree(bool ismmap)
{
  struct ainfo *ai,*aibas;
  ub4 a,len,n,cnt=0;
  ub4 y;
  ub2 allan;

  if (ismmap) {
    aibas = aitab2;
    len = 1U << Aihshbit2;
  } else {
    aibas = aitab1;
    len = 1U << Aihshbit1;
  }

  for (a = 0; a < len; a++) {
    ai = aibas + a;
    if (ai->ptr && ai->freeanchor == hi16) {
      allan = ai->allocanchor;
      cnt++;
      y = elsizes[allan];
      n = ai->len;
      if (n > 8192) warnfln(flns[allan],"mem.%u unfreed %clk %u`B '%s'",__LINE__,ismmap ? 'B' : 'b',n,descs[allan]);
    }
  }
  return cnt;
}

void achkfree(void)
{
  showcnt("unfreed block",chkfree(0));
  showcnt("unfreed Block",chkfree(1));
}

void *alloc_fln(ub4 fln,ub4 nelem,ub4 elsiz,ub2 fil,const char *desc,ub2 counter)
{
  ub8 n8,x8;
  ub4 n,nn,nm,totalmb,align;
  ub1 *p;
  struct ainfo *ai;
  ub2 allan,mod;
  bool ismmap;

  if (nelem < mini_thres && elsiz < mini_thres) {
    if (nelem == 0 && (fil & Mo_ok0) ) return nil;
    n = nelem * elsiz;
    if (n < mini_thres) {
      align = min(elsiz,stdalign);
      p = minalloc_fln(fln,n,align,fil,desc);
      return p;
    }
  }

  align = min(elsiz,maxalign);
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

  nm = n >> 20;

  if (nm >= Maxmem_mb) {
    fatal(fln,"exceeding %u MB limit by %u %s",Maxmem_mb,nm,desc);
  }
  totalmb = totalkb >> 10;
  if (totalmb + nm >= Maxmem_mb) {
    fatal(fln,"exceeding %u MB limit by %u+%u=%u MB %s",Maxmem_mb,totalmb,nm,nm + totalmb,desc);
  }

  nn = align4(fln,n,align,desc);
  addsum(nn);
  if (nn >= mmap_thres) {
    ismmap = 1;
    infofln(fln,"mem.%u Alloc %u`B %s",__LINE__,nn,desc);
    p = osmmap(nn,ub1);
    if (!p) fatal(fln,"cannot alloc %u`B, total %u MB for %s: %m",nn,totalmb,desc);
    if (fil && fil < Mnofil) { info("mem.%u set %x",__LINE__,fil); memset(p,fil,nn); }
  } else {
    ismmap = 0;
    vrbfln(fln,"mem.%u alloc %u`B %s",__LINE__,nn,desc);
    p = malloc(nn);
    if (!p) fatal(fln,"cannot alloc %u`B, total %u MB for %s: %m", nn,totalmb,desc);
    if (fil < Mnofil) { info("mem.%u set %x",__LINE__,fil); memset(p,fil,nn); }
    x8 = (ub8)p & pagemask;
    if (x8 <= maxalign) return p; // not freed
  }

  ai = getai(p,ismmap,1);
  if (ai == nil) return p;

  mod = fln >> 16;
  if (mod > Shsrc_mem) return p;

  allan = mod * Memdesc + counter;
  descs[allan] = desc;
  flns[allan] = fln;
  if ( (n = elsizes[allan]) && n != elsiz) fatal(fln,"elsize %u vs %u %s",n,elsiz,desc);
  elsizes[allan] = elsiz;

  ai->len = nn;
  ai->allocanchor = allan;
  ai->freeanchor = hi16;

  return p;
}

static inline bool is_mmapped(ub8 p)
{
  p &= pagemask;

  return (p <= maxalign);
}

void afree_fln(ub4 fln,const void *p,const char *desc,ub2 counter)
{
  struct ainfo *ai;
  ub4 elsiz;
  ub4 len;
  ub8 x8;
  ub2 allan,freean,anchor,mod;
  bool ismmap;

  if (desc == nil) desc = "(nil)";

  if (!p) { errorfln(fln,FLN,"free nil pointer for %s",desc); return; }
  x8 = (ub8)p;
  if (x8 & minalignmask) return; // minpool

  ismmap = is_mmapped(x8);

  ai = getai(p,ismmap,0);
  if (ai) {
    freean = ai->freeanchor;
    allan = ai->allocanchor;
    elsiz = elsizes[allan];
    len   = ai->len;

    if (freean != hi16) {
      infofln(flns[freean],"mem.%u location of previous free",__LINE__);
      errorfln(fln,FLN,"double free of pointer %p '%s'",p,desc);
      infofln(flns[allan],"mem.%u location of allocation '%s'",__LINE__,descs[allan]);
      return;
    }

    mod = fln >> 16;
    if (mod > Shsrc_mem) return;
    anchor = mod * Memdesc + counter;
    descs[anchor] = desc;
    flns[anchor] = fln;
    elsizes[anchor] = elsiz;
    ai->freeanchor = anchor;
  } else return;

  subsum(len);
  if (ismmap) {
    osmunmap(p,len);
  } else free((void *)p);
}

void afree0_fln(ub4 fln,void *p,const char *desc,ub2 counter)
{
  if (p) afree_fln(fln,p,desc,counter);
}

void *allocset_fln(ub4 fln,struct mempart *parts,ub2 npart,ub2 fil,const char *desc,ub2 counter)
{
  ub4 len=0,len2;
  ub4 nel,siz;
  ub2 align,align0=1;
  ub2 f;
  ub2 part;
  char *bas,*p;
  bool ismmap;

  for (part = 0; part < npart; part++) {
    nel = parts[part].nel;
    if (nel == 0) continue;
    siz = parts[part].siz;
    if (siz == 0) ice(fln,hi32,"part %u nil elsiz for cnt %u %s",part,nel,desc);
    align = min(siz,16);
    if (len == 0) {
      align0 = align;
    } else len = align4(fln,len,align,desc);
    len += nel * siz;
  }

  len2 = (len + align0) / align0;
  bas = alloc_fln(fln,len2,align0,fil,desc,counter);
  ismmap = is_mmapped((ub8)bas);

  len = 0;
  for (part = 0; part < npart; part++) {
    nel = parts[part].nel;
    if (nel == 0) continue;
    siz = parts[part].siz;
    align = min(siz,16);
    len = align4(fln,len,align,desc);
    parts[part].ptr = p = bas+len;
    f = parts[part].fil;
    if (fil == Mnofil && f != Mnofil) {
      if (f || ismmap == 0) memset(p,f,nel * siz);
    }
    len += nel * siz;
  }
  return bas;
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
      bas = minalloc_fln(fln,ncnt * elsiz,align,0,"blkexp");
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
  pagemask = ospagesize - 1;
}

void eximem(bool show)
{
  vrb("max ai use %u,%u",aiuses[0],aiuses[1]);
  if (show & globs.resusg) info("max net   mem use %lu`B",(ub8)maxkb << 10);
}
