/* util.c - generic utility functions

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

/* This file contains utiity functions that do not deserve their own place

 */

#include <stdarg.h>
#include <string.h>

#include "base.h"
#include "os.h"

#include "fmt.h"

static ub4 msgfile = Shsrc_util;
#include "msg.h"

#include "mem.h"
#include "util.h"

#include "tim.h"

#if defined __clang__
 #pragma clang diagnostic ignored "-Wsign-conversion"
#elif defined __GNUC__
 #pragma GCC diagnostic ignored "-Wsign-conversion"
 #pragma GCC diagnostic ignored "-Wpointer-sign"
#endif

static bool memeq(cchar *p,cchar *q,ub2 n)
{
  ub2 i = 0;
  while (i < n && p[i] == q[i]) i++;
  return i == n;
}

static bool streq(cchar *p,cchar *q)
{
  if (!p || !q || (*p | *q) == 0) return 0; // do not accept both empty

  while (*p) {
    if (*p != *q) return 0;
    p++; q++;
  }
  return (*q == 0);
}

static inline ub4 strlen4(cchar *s) { return (ub4)strlen(s); }

ub8 gettime_msec(void)
{
  ub8 usec = gettime_usec();
  return usec / 1000;
}

#if 0
int filecreate(const char *name,int mustsucceed)
{
  int fd = oscreate(name);
  if (fd == -1) {
    if (mustsucceed) oserror("cannot create %s",name);
  }
  return fd;
}

int fileappend(const char *name)
{
  int fd = osappend(name);
  if (fd == -1) oserror("cannot create or append %s",name);
  return fd;
}

int fileopenfln(ub4 fln,const char *name,int mustexist)
{
  int fd = osopen(name);
  if (fd == -1) {
    if (mustexist > 0) oserrorfln(fln,"cannot open %s",name);
  }
  return fd;
}

int filewritefln(ub4 fln,int fd, const void *buf,ub8 len,const char *name)
{
  long n;

  if (len == 0) { warning("nil write to %s",name); return 0; }
  if (fd == -1) { errorfln(fln,FLN,"write to invalid handle for %s",name); return 1; }
  n = oswrite(fd,buf,len);
  if (n == -1) {
    osclose(fd);
    oserrorfln(fln,"cannot write %lu` bytes to %s",len,name);
    return 1;
  }
  else if (n != (long)len) {
   osclose(fd);
   errorfln(fln,FLN,"partial write %ld` of %lu` bytes to %s",n,len,name);
   return 1;
  }
  else return 0;
}

int fileread(int fd,void *buf,ub4 len,const char *name)
{
  long n;

  if (len == 0) { error("nil read from %s",name); return 1; }

  n = osread(fd,buf,len);

  if (n == -1) { oserror("cannot read from %s",name); return 1; }
  else if (n == 0) { error("eof on reading %u`B from %s",len,name); return 1; }
  else if (n != (long)len) { error("partial read %ld` of %u`B from %s",n,len,name); return 1; }
  else return 0;
}

int fileclose(int fd,const char *name)
{
  int rv = osclose(fd);
  if (rv) oserror("cannot close %s",name);
  return rv;
}
#endif

int fileexists(const char *name)
{
  int rv = osexists(name);
  if (rv == -1) { oserror("cannot stat %s",name); return -1; }
  else return rv;
}

int fileremove(const char *name)
{
  int rv = osremove(name);
  if (rv == -1) { oserror("cannot remove %s",name); return -1; }
  else return rv;
}

// truncate at integral utf-8 char
ub4 truncutf8(const char *s,ub4 len)
{
  while (len && (s[len-1] & 0xc0) == 0x80) len--;
  if (len && (s[len-1] & 0xc0) == 0xc0) len--;
  return len;
}

#if 0
// adapted from cs.clackamas.cc.or.us/molatore/cs260Spr03/combsort.htm
static ub4 combsort8(ub8 *p,ub4 n,int partial)
{
  ub4 iter = 0;
  ub4 gap = n;
  ub8 v0,v1;
  int swap;
  static ub4 gaps[7] = {11,8,6,4,3,2,1};
  ub4 i,gi = 0;

  do { // gapped stage
    if (gap > 14) gap = (gap * 10) / 12;
    else gap = gaps[gi++];
    iter++;
    for (i = 0; i + gap < n; i++) {
      v0 = p[i]; v1 = p[i+gap];
      if (v0 > v1) { p[i] = v1; p[i+gap] = v0; }
    }
  } while (gap > 1);
  if (partial) return iter;

  do { // final bubble stage
    swap = 0;
    iter++;
    for (i = 1; i < n; i++) {
      v0 = p[i-1]; v1 = p[i];
      if (v0 > v1) {
        p[i-1] = v1; p[i] = v0;
        swap = 1;
      }
    }
  } while (swap);
  return iter;
}

static ub4 combsort4(ub4 *p,ub4 n)
{
  ub4 iter = 0;
  ub4 gap = n;
  ub4 v0,v1;
  int swap;
  static ub4 gaps[7] = {11,8,6,4,3,2,1};
  ub4 i,gi = 0;

  do { // gapped stage
    if (gap > 14) gap = (gap * 10) / 12;
    else gap = gaps[gi++];
    iter++;
    for (i = 0; i + gap < n; i++) {
      v0 = p[i]; v1 = p[i+gap];
      if (v0 > v1) { p[i] = v1; p[i+gap] = v0; }
    }
  } while (gap > 1);

  do { // final bubble stage
    swap = 0;
    iter++;
    for (i = 1; i < n; i++) {
      v0 = p[i-1]; v1 = p[i];
      if (v0 > v1) {
        p[i-1] = v1; p[i] = v0;
        swap = 1;
      }
    }
  } while (swap);
  return iter;
}

// simple unchecked version.
ub4 fsort8(ub8 *p,ub4 n,int partial,ub4 fln,const char *desc)
{
  ub4 i,rv = 1;
  ub8 v;

  switch (n) {  // trivia
  case 0: return warnfln(fln,"sort of nil items for %s",desc);
  case 1: return 0;
  case 2: v = p[0]; if (v > p[1]) { p[0] = p[1]; p[1] = v; } return 1;
  }

  for (i = 1; i < n; i++) if (p[i] < p[i-1]) break;
  if (i == n) return 0; // already sorted

  for (i = 1; i < n; i++) if (p[i] > p[i-1]) break;
  if (i == n) { // already reverse sorted
    for (i = 0; i < n / 2; i++) {
      v = p[i]; p[i] = p[n-i-1]; p[n-i-1] = v;
    }
  } else rv = combsort8(p,n,partial);

  return rv;
}

ub4 sort4(ub4 *p,ub4 n,ub4 fln,const char *desc)
{
  ub4 i,rv = 1;
  ub4 v;

  switch (n) {  // trivia
  case 0: return warnfln(fln,"sort of nil items for %s",desc);
  case 1: return 0;
  case 2: v = p[0]; if (v > p[1]) { p[0] = p[1]; p[1] = v; } return 1;
  }

  for (i = 1; i < n; i++) if (p[i] < p[i-1]) break;
  if (i == n) return 0;

  for (i = 1; i < n; i++) if (p[i] > p[i-1]) break;
  if (i == n) {
//    vrbfln(fln,"csort of %u started in reverse order",n);
    for (i = 0; i < n / 2; i++) {
      v = p[i]; p[i] = p[n-i-1]; p[n-i-1] = v;
    }
  } else rv = combsort4(p,n);

  for (i = 1; i < n; i++) if (p[i] < p[i-1]) break;
  if (i < n) {
    warnfln(fln,"csort of %u not in order in %u runs",n,rv);
    for (i = 0; i < n; i++) { infofln(fln,0,"%u %u",i,p[i]); }
  }
  return rv;
}
#endif

#if 1
// variant binary search, after en.wikipedia.org/wiki/Interpolation_search
// returns nearest position if no match
ub4 bsearch4(ub4 *p,ub4 n,ub4 key,ub4 fln)
{
  ub4 lo,hi,mid,i;
  ub4 v;
  ub4 iter = 0;

  switch (n) {  // trivia
  case 0: warnfln(fln,"search in nil items"); return 0;
  case 1: return *p == key ? 0 : 1;
  case 2: if (p[0] >= key) return 0;
          else if (p[1] >= key) return 1;
          else return 2;
  case 3: if (p[0] >= key) return 0;
          else if (p[1] >= key) return 1;
          else if (p[2] >= key) return 2;
          else return 3;
  case 4: case 5: case 6: case 7: case 8: case 9: case 10:
          i = 0;
          while (i < n) {
            if (p[i] >= key) return i;
            i++;
          }
          return n;
  }

  lo = 0; hi = n - 1;
  if (key < p[lo]) return 0;
  else if (key > p[hi]) return n;

  mid = 0;
  while (hi - lo > 1) {
    if (iter > 200) {
      warnfln(fln,"bsearch iter %u cnt %u lo %u hi %u",iter,n,lo,hi);
      return mid;
    }
    iter++;
    mid = lo + (hi - lo) / 2;
    v = p[mid];
    if(v == key) break;
    else if (v < key) lo = mid;
    else hi = mid;
  }
//  infofln(fln,"bsearch iter %u cnt %u lo %u hi %u %u",iter,n,lo,hi,p[mid]);
  return mid; // nearest
}
#endif

#if 0
// see above
ub4 bsearch8(ub8 *p,ub4 n,ub8 key,ub4 fln,const char *desc)
{
  ub4 lo,hi,mid,i;
  ub8 v;
  ub4 iter = 0;

  switch (n) {  // trivia
  case 0: return warnfln(fln,"search in nil items for %s",desc);
  case 1: return *p == key ? 0 : 1;
  case 2: if (p[0] >= key) return 0;
          else if (p[1] >= key) return 1;
          else return 2;
  case 3: if (p[0] >= key) return 0;
          else if (p[1] >= key) return 1;
          else if (p[2] >= key) return 2;
          else return 3;
  case 4: case 5: case 6: case 7: case 8: case 9: case 10:
          i = 0;
          while (i < n) {
            if (p[i] >= key) return i;
            i++;
          }
          return n;
  }

  lo = 0; hi = n - 1;
  if (key < p[lo]) return 0;
  else if (key > p[hi]) return n;

  mid = 0;
  while (hi - lo > 1) {
    if (iter > 200) {
      warnfln(fln,"bsearch iter %u cnt %u lo %u hi %u",iter,n,lo,hi);
      return mid;
    }
    iter++;
    mid = lo + (hi - lo) / 2;
    v = p[mid];
    if(v == key) break;
    else if (v < key) lo = mid;
    else hi = mid;
  }
  infofln(fln,"bsearch iter %u cnt %u lo %u hi %u",iter,n,lo,hi);
  return mid; // nearest
}
#endif

// read a file, allocating its length
int readfile_pad(struct myfile *mf,const char *name, int mustexist,ub4 maxlen,ub4 padlen,const char pad)
{
  int rv;
  int fd;
  ub1 *buf;
  ub4 len,alen;
  ub4 nr;
  struct osstat ino;

  mf->len = 0;
  mf->alloced = 0;
  mf->exist = 0;

  if (name == nil || *name == 0) { errorfln(FLN,0,"empty filename %p passed to readfile",(const void *)name); return 1; }
  if (name != mf->name) strncpy(mf->name,name,sizeof(mf->name)-1);
  fd = osopen(name);
  if (fd == -1) {
    if (mustexist) { oserrorfln(FLN,"cannot open %s",name); return 1; }
    else return 0;
  }
  if (osfdinfo(&ino,fd)) { oserrorfln(FLN,"cannot get info for %s",name); osclose(fd); return 1; }
  mf->exist = 1;
  if (padlen > hi16) { errorfln(FLN,mf->fln,"%s padding exceeds %u",name,hi16); return 1; }

  mf->mtime = ino.mtime;

  if (maxlen == 0) maxlen = hi32;
  if (maxlen > hi16) maxlen -= padlen;
  if (ino.len > maxlen) { errorfln(FLN,mf->fln,"%s length %lu exceeds %u",name,ino.len,maxlen); return 1; }

  len = (ub4)ino.len;
  mf->len = len;

  if (len == 0) {
    osclose(fd);
    info("%s is empty",name);
    return 0;
  }

  alen = len + padlen;
  mf->xlen = alen;
  buf = alloc(alen,ub1,0,mf->name,nextcnt);
  mf->alloced = 1;

  mf->buf = buf;
  rv = osread(fd,(char *)buf,len,&nr);
  if (rv) {
    if (rv < 0) oserror("cannot read %s",name);
    else error("cannot read %s",name);
    osclose(fd);
    return 1;
  }
  osclose(fd);
  if (nr != len) { error("read %u`B of %u`B of %s",(ub4)nr,(ub4)len,name); return 1; }
  if (padlen) memset(buf + len,pad,padlen);
  return 0;
}

int readfile(struct myfile *mf,const char *name, int mustexist,ub4 maxlen) {
  return readfile_pad(mf,name,mustexist,maxlen, 0, 0);
}

int freefile(struct myfile *mf)
{
  if (mf->alloced == 0) return 0;

  afree(mf->buf,mf->name,nextcnt);
  mf->alloced = 0;
  mf->buf = nil;
  return 0;
}

int filebck(cchar *name)
{
  char bck[Fname];
  int rv = 0;

  if (osexists(name) == 1) {
    snprintf(bck,Fname,"%s.bck",name);
    rv = osrename(name,bck);
    if (rv) oserror("cannot rename %s to %s",name,bck);
  }
  return rv;
}

#if 0
int writefile(const char *name,char *buf,size_t len)
{
  int fd = filecreate(name,1);

  if (fd == -1) return 1;

  if (filewrite(fd,buf,len,name)) return 1;
  return fileclose(fd,name);
}

int readpath(struct myfile *mf,const char *dir,const char *name, int mustexist,ub4 maxlen)
{
  char fname[1024];

  if (dir == NULL || *dir == 0) return readfile(mf,name,mustexist,maxlen);
  fmtstring(fname,"%s/%s",dir,name);
  return readfile(mf,fname,mustexist,maxlen);
}
#endif

static ub4 myflush(struct bufile *f,ub4 n,bool eof)
{
  int fd = f->fd;
  struct myfile mf;
  ub4 len;
  cchar *p;

  if (n == 0 || f->err) return 0;

  if (fd == -1) {
    if (f->dobck) {
      if (f->dobck == 2 && eof) {
        if (readfile(&mf,f->nam,0,hi16)) return 0;
        else if (mf.exist) {
          len = mf.len;
          p = mf.buf;

          if (len > 16 && len == n && memcmp(f->buf,p,n) == 0) return 0; // will be equal
        }
      }
      filebck(f->nam);
    }
    fd = oscreate(f->nam);
    if (fd == -1) { f->err = 1; return 0; }
    f->fd = fd;
  }
  if (oswrite(fd,f->buf,n)) f->err = 1;
  return n;
}

// creates file at first buffer flush
void myfopen(struct bufile *f,ub4 len,bool perm)
{
  ub1 bit;

  len = nxpwr2(min(len,1U << 20),&bit);
  if (len > 16) len -= 16;
  f->mid = len;

  len *= 2;
  f->top = len;

  f->perm = perm;
  f->buf = alloc(len,ub1,Mo_nofill,f->nam,nextcnt);
  f->fd = -1;
}

ub4 myfwrite(struct bufile *f,ub1 *src,ub4 n)
{
  ub4 pos = f->pos;
  ub4 mid = f->mid;
  ub4 top = f->top;
  ub4 nw;
  ub1 *buf = f->buf;

  if (buf == nil) ice(0,hi32,"write to unopened %s",f->nam);

  if (pos >= mid) {
    n = myflush(f,mid,0);
    if (f->err) return n;
    n = pos - mid;
    if (n) memcpy(buf,f->buf + mid,n);
    pos = n;
  }
  nw = min(n,top - pos - 4);
  memcpy(buf + pos,src,nw);
  f->pos = pos;

  return nw;
}

void myfputc(struct bufile *f,ub1 c)
{
  ub4 pos = f->pos;
  ub4 mid = f->mid;
  ub4 n;
  ub1 *buf = f->buf;

  if (buf == nil) ice(0,hi32,"write to unopened %s",f->nam);

  if (pos >= mid) {
    myflush(f,mid,0);
    if (f->err) return;
    n = pos - mid;
    if (n) memcpy(buf,f->buf + mid,n);
    pos = n;
  }
  buf[pos] = c;
  f->pos = pos + 1;
}

void myfputs(struct bufile *f,cchar *s,ub2 len)
{
  ub4 pos = f->pos;
  ub4 mid = f->mid;
  ub4 n;
  ub1 *buf = f->buf;

  if (buf == nil) ice(0,hi32,"write to unopened %s",f->nam);

  if (pos >= mid) {
    myflush(f,mid,0);
    if (f->err) return;
    n = pos - mid;
    if (n) memcpy(buf,f->buf + mid,n);
    pos = n;
  }
  if (len > mid) len = mid;
  memcpy(buf + pos,s,len);
  f->pos = pos + len;
}

ub4 __attribute__ ((format (printf,2,3))) myfprintf(struct bufile *f,const char *fmt,...)
{
  va_list ap;
  ub4 n,nn;
  ub4 pos = f->pos;
  ub4 mid = f->mid;
  ub4 top = f->top;
  ub1 *buf = f->buf;

  if (buf == nil) ice(0,hi32,"write to unopened %s",f->nam);

  if (fmt == nil || *fmt == 0 || f->err) return 0;

  if (pos >= mid) {
    n = myflush(f,mid,0);
    if (f->err) return n;
    n = pos - mid;
    if (n) memcpy(buf,f->buf + mid,n);
    pos = n;
  }

  va_start(ap, fmt);
  nn = myvsnprint(buf+pos,0,top-pos-4,fmt,ap);
  va_end(ap);
  f->pos = pos + nn;
  return nn;
}

int myfclose(struct bufile *f)
{
  if (f->err) return 1;

  if (f->pos) myflush(f,f->pos,1);
  if (f->fd != -1) osclose(f->fd);
  f->fd = -1;
  if (f->perm == 0) afree(f->buf,f->nam,nextcnt);
  return f->err;
}

ub4 *mklntab(cchar *p,ub4 n,ub4 *pcnt)
{
  ub4 i;
  ub4 cnt = 0;
  ub4 *tab;

  *pcnt = 0;
  if (n < 3) return nil;

  for (i = 0; i < n; i++) if (p[i] == '\n') cnt++;
  if (cnt == 0) return nil;
  cnt++;

  // todo
  if (cnt < 1024) tab = minalloc(cnt * 4,4,Mo_nofill,"lintab");
  else tab = medalloc(cnt * 4,4,"lintab");

  cnt = 0;
  for (i = 0; i < n; i++) {
    if (p[i] == '\n') tab[cnt++] = i;
  }
  tab[cnt] = n+1;
  *pcnt = cnt;
  return tab;
}

ub4 parse_version(cchar *p,ub1 len)
{
  ub4 vs[4],z = 0;
  ub2 x = 0,n = 0,bit = 4;
  ub4 ret;

  vs[0] = vs[1] = vs[2] = 0;

  while (n < 3 && len--) {
    if (*p == '.') { vs[n++] = x; x = 0; bit >>= 1; }
    else if (*p >= '0' && *p <= '9') { x = x * 10 + (*p - '0'); }
    else if (*p == '+') { vs[n] = x; z |= bit; break; }
    else if (*p == '-') { vs[n] = x; break; }
    p++;
  }
  vs[n] = x;
  ret = (z << 24) | (vs[0] << 16) | (vs[1] << 8) | vs[2];
  return ret;
}

// returns nonzero on match
ub1 check_version(ub4 chk,ub4 ver)
{
  ub1 z = (chk >> 24);
  ub1 v0,v1,v2,c0,c1,c2;

  v0 = ver >> 16;
  v1 = ver >> 8;
  v2 = ver;

  c0 = chk >> 16;
  c1 = chk >> 8;
  c2 = chk;

  // major
  if (v0 < c0) return 0;
  else if (v0 > c0) return z & 4;
  else {
    if (z & 4) return 1;
    if (v1 < c1) return 0;
    else if (v1 > c1) return z & 2;
    else {
      if (z & 2) return 1;
      if (v2 < c2) return 0;
      else if (v1 > c1) return z & 1;
      else return 1;
    }
  }
}

static void show_version(bool full)
{
  ub4 len = 1024;
  char buf[1024];
  ub4 pos;

  iniprgtim();

  pos = mysnprintf(buf,0,len,"\n%s %u.%u %s %s\n",
    globs.prgnam,Version_maj,Version_min,Version_phase,fmtdate(globs.prgdtim,hi32));

 if (full) {

  pos += mysnprintf(buf,pos,len,"  compiled with ");

#if defined __clang__ && defined __clang_major__ && defined __clang_minor__

  pos += mysnprintf(buf,pos,len,"clang %u.%u",__clang_major__,__clang_minor__);

#elif defined __GNUC__ && defined __GNUC_MINOR__

  pos += mysnprintf(buf,pos,len,"gcc %u.%u",__GNUC__,__GNUC_MINOR__);

  #ifdef __CET__
    pos += mysnprintf(buf,pos,len," +cfp");
  #endif

#else
  fmtstring(ccstr,"unknown compiler");
#endif

 #ifdef Asan
    pos += mysnprintf(buf,pos,len," +asan");
 #endif

 }

  buf[pos++] = '\n';
  msg_write(buf,pos);
}

static void show_license(void)
{
  static cchar lictxt[] = "  This file is part of Morelia, a subset of Python with emphasis on efficiency.\n\
\n\
  Copyright © 2022 Joris van der Geer.\n\
\n\
  Morelia is free software: you can redistribute it and/or modify\n\
  it under the terms of the GNU Affero General Public License as published by\n\
  the Free Software Foundation, either version 3 of the License, or\n\
  (at your option) any later version.\n\
\n\
  Morelia is distributed in the hope that it will be useful,\n\
  but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
  GNU General Public License for more details.\n\
  \n\
  You should have received a copy of the GNU Affero General Public License\n\
  along with this program, typically in the file License.txt\n\
  If not, see http://www.gnu.org/licenses.\n\n";

  show_version(0);

  msg_write(lictxt,sizeof(lictxt)-1);
}

static ub2 hilongopt;

static struct cmdopt genopts[] = {
 { "help",    'h', Co_help,    nil,         "Show usage and quit" },
 { "license", 'L', Co_license, nil,         "Show license and quit" },
 { "",        '?', Co_help,    nil,         "Show help and quit" },
 { "dryrun",  'n', Co_dry,     nil,         "Do not execute" },
 { "quiet",   'q', Co_quiet,   nil,         "Suppress informal output" },
 { "silent",  ' ', Co_quiet,   nil,         "Suppress informal output" },
 { "rusage",  'r', Co_resusg,  nil,         "Show resource usage" },
 { "version", 'V', Co_version, nil,         "Show version and quit" },
 { "verbose", 'v', Co_verbose, "?%ulevel", "Verbose level" },
 { "",        'e', Co_errwarn, nil,         "Treat warnings as error" },
 { nil,0,0,nil,nil}
};

static ub4 usgline(ub4 pos,char *buf,ub4 len,const char sname, const char *lname,struct cmdopt *op)
{
  char c0,c1,c2,c3,c4;
  cchar *arg;

  if (sname != ' ') { c0 = '-'; c1 = sname; c2 = ','; }
  else { c0 = c1 = c2 = ' '; }

  if (lname && *lname) c3 = c4 = '-';
  else { c3 = c4 = ' '; lname = ""; c2 = ' '; }

  pos += mysnprintf(buf,pos,len,"  %c%c%c",c0,c1,c2);
  pos += mysnprintf(buf,pos,len,"  %c%c%-*s",c3,c4,hilongopt + 2,lname);

  arg = op->arg;
  if (arg) {
    if (*arg == '?') arg++;
    if (*arg == '%' && arg[1]) arg += 2;
  } else arg = "";
  pos += mysnprintf(buf,pos,len,"%-8s %s\n",arg,op->desc);
  return pos;
}

void usage(struct cmdopt *opts)
{
  struct cmdopt *op = opts,*gop = genopts;
  char s1,s2;
  const char *l1;
  const char *l2;
  bool line1,line2;
  ub4 pos;
  ub4 len = 4096;
  char buf[4096];

  while (op->opt) op++;
  pos = mysnprintf(buf,0,len,"\n%s\n\nUsage: %s [options]",op->desc,globs.prgnam);
  if (op->arg && op->arg[0]) pos += mysnprintf(buf,pos,len," %s  (%u arg%.*s)\n\nOptions:\n",
    op->arg,op->sname,op->sname,"s");

  op = opts;
  while (op->opt | gop->opt) {
    if (op->opt) {
      s1 = op->sname;
      l1 = op->lname;
    } else {
      s1 = 0;
      l1 = nil;
    }
    if (gop->opt) {
      s2 = gop->sname;
      l2 = gop->lname;
    } else {
      s2 = 0;
      l2 = nil;
    }

    line1 = line2 = 0;

    if (s1 && s2) {
      if (s1 < s2) line1 = 1;
      else if (s1 == s2) line1 = line2 = 1;
      else line2 = 1;
    }
    else if (s1) line1 = 1;
    else if (s2) line2 = 1;

    else if (l1 && l2 && *l1 && *l2) {
      if (*l1 < *l2) line1 = 1;
      else if (*l1 == *l2 && streq(l1,l2)) line1 = line2 = 1;
      else line2 = 1;
    }
    else if (l1 && *l1) line1 = 1;
    else if (l2 && *l2) line2 = 1;
    else break;

    if (line1) pos = usgline(pos,buf,len,s1,l1,op++);
    else if (line2) pos = usgline(pos,buf,len,s2,l2,gop);
    if (line2) gop++;
    if (pos > len / 2) { msg_write(buf,pos); pos = 0; }
  }
  if (pos) msg_write(buf,pos);
}

static ub2 genlut[256];
static const ub2 lutinit = 0xaa55;

void prepopts(struct cmdopt *opts,ub2 *lut,bool clr)
{
  ub2 c,l;
  const unsigned char *lp;
  ub4 n,ndx = 0;

  if (clr) memset(lut,0,256 * sizeof(ub2));
  lut[0] = lutinit;
  while(opts[ndx].opt) {
    c = opts[ndx].sname;
    if (c != ' ') lut[c] = ndx | 0x4000;
    lp = (const unsigned char *)opts[ndx].lname;
    if (lp && *lp) {
      l = *lp;
      lut[l] |= 0x8000;
      n = strlen4(lp);
      if (n > hilongopt) hilongopt = n;
    }
    ndx++;
    if (ndx == 0x4000) { error("exceeding max %u cmdline options",ndx); return; }
  }
}

static struct cmdopt *findsopt(const unsigned char s,struct cmdopt *opts,ub2 *lut)
{
  struct cmdopt *op = opts;
  ub2 x;
  char c;

  if (lut && lut[0] == lutinit && s) {
    x = lut[(ub2)s] & 0x7fff;
    if (x & 0x4000) return opts + (x & 0x3fff);
  }
  while(op->opt) {
    c = op->sname;
    if (c == s) return op;
    op++;
  }
  return nil;
}

static void getuarg(cchar *p,struct cmdval *cv)
{
  ub4 x=0;
  char c;

  while ((c = *p++)) {
    cv->vlen++;
    if (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); }
    else if (c) cv->err = FLN;
    if (x > hi32 / 16) { cv->err = FLN; return; }
  }
  cv->uval = x;
}

static void getearg(cchar *ap,cchar *lp,struct cmdval *cv)
{
  ub4 x=0;
  ub4 llen = strlen4(lp);
  ub4 len;
  cchar *nx;

  if (*lp >= '0' && *lp <= '9') { getuarg(lp,cv); return; }
  cv->err = FLN;
  while(*ap) {
    nx = strchr(ap,',');
    if (nx == nil) { len = strlen4(ap); nx = ap + len-1; }
    else len = (ub4)(nx - ap);
    if (len == llen && memeq(lp,ap,len)) { cv->uval = x; cv->err = 0; cv->vlen = len; return; }
    x++; ap = nx+1;
  }
}

enum Optargs { Optnone, Optional, Optreq, Optstr=0x10, Optint=0x20,Optenum=0x40 };

static void optarg(const char *ap,const char *lp,struct cmdval *cv)
{
  if (*ap != '%') {
    cv->sval = lp;
    cv->vlen = strlen(lp);
    return;
  }
  ap++;

  switch (*ap) {
    case 'u': getuarg(lp,cv); break; // uint
    case 'e': getearg(ap+1,lp,cv); break; // enum
    default: warnfln(0,"unknown arg type %c",*ap); cv->err = FLN;
  }
}

static struct cmdopt *findlopt(const unsigned char *lpp,struct cmdopt *opts,struct cmdval *cv)
{
  struct cmdopt *op = opts;
  const char *l;
  const char *ap;
  const unsigned char *lp;
  enum Optargs xopt;

  while(op->opt) {
    lp = lpp;
    l = op->lname;
    if (l == nil || *l == 0) { op++; continue; }
    while (*l && *l == *lp) { l++; lp++; }
    if (*l) { op++; continue; }

    ap = op->arg;
    if (ap == nil || *ap == 0) xopt = Optnone;
    else {
      if (*ap == '?') { xopt = Optional; ap++; }
      else xopt = Optreq;
      if (*ap == '%') {
        ap++;
        switch (*ap) {
        case 'u': xopt |= Optint; ap++; break;
        case 'e': xopt |= Optenum; ap++; break;
        default : xopt |= Optstr;
        }
      }
    }

    if (*l == *lp) {
      if ( (xopt & 3) != Optreq) return op;   // no arg and none required
      else { cv->err = FLN; return op; }

    } else if (*lp == '=') {
      cv->olen = lp - lpp + 2;
      cv->sval = ++lp;
      if ( (xopt & 3) == Optnone) { cv->err = FLN; return op; } // arg given but none expected
      else {
        if (*lp == 0) { cv->err = FLN; return op; }
        if (xopt & Optint) {
          getuarg(lp,cv);
        } else if (xopt & Optenum) {
          getearg(ap,lp,cv);
        }
        return op;
      }
    }

    op++;
  }
  return nil;
}

static void do_genop(struct cmdopt *opts,struct cmdval *cv)
{
  struct cmdopt *op = cv->op;
  ub2 lvl;

  switch(op->opt) {
    case Co_help:    usage(opts);     break;
    case Co_version: show_version(1); break;
    case Co_license: show_license();  break;
    case Co_resusg:  globs.resusg = 1;break;
    case Co_verbose: if (cv->vlen) lvl = cv->uval;
                     else lvl = globs.msglvl + 1;
                     if (lvl >= 10) { setmsgbuf(0); lvl -= 10; }
                     globs.msglvl = min(Vrb2,lvl + Info);
                     info("msg lvl %u",globs.msglvl);
                     break;
    case Co_quiet:   globs.msglvl = Warn; break;
    case Co_errwarn: globs.errwarn = 1;   break;
    default:         warning("unhandled option %s",op->desc);
  }
}

static enum Parsearg do_parseargs(ub4 argc,char *argv[],struct cmdopt *opts,struct cmdval *cv,ub2 *lut,bool min1long)
{
  const char *arg;
  const unsigned char *longopt=0;
  const char *ap;
  unsigned char c,c2,c3;
  ub4 ndx=0;
  struct cmdopt *op;
  unsigned char shortopt,longopt1;
  enum Parsearg ret;

  memset(cv,0,sizeof(struct cmdval));

  if (!argc) return Pa_eof;
  arg = *argv;

  if (arg == nil) return Pa_eof;

  c = arg[ndx++];
  cv->olen = strlen(arg);
  cv->vlen = 0;

  switch(c) {
    case 0: return Pa_nil;
    case '-': break;
    case '+': break;
//    case '\'': delim = c; break;
//    case '"': delim = c; break;
    default:
      return Pa_regarg; // todo quote removal
  }

  shortopt = 0;
  longopt1 = 0;

  c2 = arg[ndx++];
  if (c2 == 0) {
    if (c == '-') return Pa_min1; // lone -
    else if (c == '+') return Pa_plus1;
    else return 0;
  } else if (c2 == '-') {
    if (c == '+') { return Pa_plusmin; } // +-
    else if (c == '-') { // --
      c3 = arg[ndx];
      if (c3 == 0) { return Pa_min2; } // lone --
      else { longopt1 = c3; longopt = (const unsigned char *)arg + ndx; }
    } else { return Pa_notfound; }
  }
  else if (arg[ndx] == 0) shortopt = c2;
  else if (min1long) { longopt1 = c2; longopt = (const unsigned char *)arg + ndx - 1; }
  else return Pa_notfound;

  if (shortopt) {
    op = findsopt(shortopt,opts,lut);
    if (op) ret = Pa_found;
    else {
      op = findsopt(shortopt,genopts,genlut);
      if (op) ret = Pa_genfound;
      else return Pa_notfound;
    }
    cv->op = op;
    cv->sval = nil;
    cv->lead = c;
    ap = op->arg;
    if (ap == nil || *ap == 0) return ret;
    if (*ap == '?') {
      ap++;
      if (argc < 2) return ret; // optional arg, none available
      else if (*ap == '%' && ap[1] == 'u' && (*argv[1] < '0' || *argv[1] > '9')) return ret;
    } else if (argc < 2) return Pa_noarg;
    optarg(ap,argv[1],cv);
    if (cv->vlen) ret++;
    return ret;

  } else if (longopt1) {
    op = nil;
    if (!lut || lut[0] != lutinit || (lut[longopt1] & 0x8000)) op = findlopt(longopt,opts,cv);
    if (op) { cv->op = op; return Pa_found; }
    if (genlut[longopt1] & 0x8000) op = findlopt(longopt,genopts,cv);
    if (op) { cv->op = op; return Pa_genfound; }
    else return Pa_notfound;
  } else return Pa_notfound;
}

enum Parsearg parseargs(ub4 argc,char *argv[],struct cmdopt *opts,struct cmdval *cv,ub2 *lut,bool min1long)
{
  enum Parsearg pa;

  pa = do_parseargs(argc,argv,opts,cv,lut,min1long);

  if (pa == Pa_genfound || pa == Pa_genfound2) {
      do_genop(opts,cv);
  }
  return pa;
}

void iniprgtim(void)
{
  globs.prgdtim = nixday2cal(globs.prgdmin / (24 * 60));
}

void iniutil(void)
{
  ub8 binmtim = osfiltim(globs.prgnam);

  globs.prgdmin = (ub4)(binmtim / 60);

  prepopts(genopts,genlut,0);
  lastcnt
}
