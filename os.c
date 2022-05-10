/* os.c - operating system specifics

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

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wreserved-id-macro"
 #pragma clang diagnostic ignored "-Wunused-macros"
#endif

// TODO check feature_test_macros

#define _POSIX_C_SOURCE 200809L

// for mremap
#define _GNU_SOURCE

// for backtrack
// #define USE_GLIBC_EXT

//#define _SVID_SOURCE
#include <sys/mman.h>

#ifdef __clang__
 #pragma clang diagnostic pop
 #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>

#ifdef USE_GLIBC_EXT
 #include <execinfo.h>
#endif

#include <string.h>
extern int rename(const char *old, const char *new);

#include <time.h>

#include "base.h"
#include "mem.h"

#include "fmt.h"

static ub4 msgfile = Shsrc_os;
#include "msg.h"

#include "os.h"

ub4 ospagesize = 4096; // default

#undef fatal
#define fatal(fln,fmt,...) fatalfln(FLN,fln,hi32,fmt,__VA_ARGS__)

static ub4 errcnt;

static volatile int sig_alrm,sig_vtalrm;

Noret void doexit(int code)
{
  eximsg();
  _exit(code);
}

int oscreate(const char *name)
{
  int rv,fd = open(name,O_CREAT|O_RDWR|O_TRUNC,0644);
  if (fd == -1) return fd;

  rv = posix_fadvise(fd,0,0,POSIX_FADV_SEQUENTIAL);
  if (rv) warning("fadvise returned %d for %s",rv,name);
  return fd;
}

int osopen(const char *name)
{
  int fd = open(name,O_RDONLY);
  if (fd == -1 && errno != ENOENT) oserror("cannot open %s",name);
  return fd;
}

int osopenseq(const char *name,int *pfd)
{
  int e;
  int rv,fd = open(name,O_RDONLY);

  *pfd = fd;
  if (fd == -1) {
    e = errno;
    if (e == ENOENT) return 1;
    oserror("cannot open %s",name);
    return -1;
  }
  rv = posix_fadvise(fd,0,0,POSIX_FADV_SEQUENTIAL);
  if (rv) warning("fadvise returned %d for %s",rv,name);
  return 0;
}

int osappend(const char *name)
{
  int fd = open(name,O_CREAT|O_WRONLY|O_APPEND,0644);
  return fd;
}

int osfdinfo(struct osstat *sp,int fd)
{
  struct stat ino;

  if (fstat(fd,&ino)) return 1;
  sp->mtime = (ub8)ino.st_mtime;
  sp->len = (ub8)ino.st_size;
  sp->ino = ino.st_ino;
  sp->dev = ino.st_dev;
  return 0;
}

ub8 osfiltim(cchar *nam)
{
  struct stat ino;

  if (nam == nil || stat(nam,&ino)) {
    oserror("cannot stat %s",nam);
    return 0;
  }
  return (ub8)ino.st_mtime;
}

#if 0
static int fileinfo(struct mysfile *mf,const char *name)
{
  struct stat ino;

  if (stat(name,&ino)) return 1;
  mf->mtime = ino.st_mtime;
  mf->len = (ub4)min(ino.st_size,hi32);
  mf->isdir = S_ISDIR(ino.st_mode);
  mf->isfile = S_ISREG(ino.st_mode);
  return 0;
}

int osfileinfo(struct myfile *mf,const char *name)
{
  struct mysfile smf;

  if (fileinfo(&smf,name)) return 1;
  mf->mtime = smf.mtime;
  mf->len = smf.len;
  mf->isdir = smf.isdir;
  mf->isfile = smf.isfile;
  return 0;
}

int osfileinfos(struct mysfile *mf,const char *name)
{
  return fileinfo(mf,name);
}
#endif

int osexists(const char *name)
{
  struct stat ino;

  if (stat(name,&ino) == 0) return 1;
  if (errno == ENOENT) return 0;
  else return -1;
}

int osrename(const char *old,const char *new)
{
  int rv = rename(old,new);
  return rv;
}

int osrewind(int fd)
{
  off_t rv;

  rv = lseek(fd,0,SEEK_SET);
  return (rv == -1 ? 1 : 0);
}

int oswrite(int fd,const char *buf,ub4 len)
{
  ssize_t nw = write(fd,buf,len);
  if (nw < len) { errcnt++; return 1; }
  else return 0;
}

int oswrite8(int fd, const char *buf,size_t len,unsigned long *nwrit)
{
  ssize_t rx;
  size_t nw=0,chk;

  *nwrit = 0;
  while (len) {
    if (fd > 2 && globs.sigint) return 1;
    chk = min(len,1024UL * 1024 * 1024);
    do rx = write(fd,buf,chk); while (rx == -1 && errno == EINTR);
    if (rx == -1) return -1;
    else if (rx == 0) return 1;
    nw += (size_t)rx;
    buf += rx;
    len -= (size_t)rx;
  }
  *nwrit = nw;
  return 0;
}

int osread8(int fd,char *buf,size_t len,unsigned long *nread)
{
  ssize_t rx;
  size_t nr=0,chk;
  *nread = 0;

  while (len) {
    if (globs.sigint) return 1;
    chk = min(len,1024UL * 1024 * 1024);
    do rx = read(fd,buf,chk); while (rx == -1 && errno == EINTR);
    if (rx == -1) return -1;
    else if (rx == 0) return 1;
    nr += (size_t)rx;
    buf += rx;
    len -= (size_t)rx;
  }
  *nread = nr;
  return 0;
}

int osread(int fd,char *buf,ub4 len,ub4 *nread)
{
  ssize_t rx = 0;

  *nread = 0;
  if (fd == -1) return 1;
  else if (len == 0) return 0;

  rx = read(fd,buf,len);
  if (rx == -1) return -1;
  else if (rx == 0) return 1;

  *nread = (ub4)rx;

  return 0;
}

int osclose(int fd)
{
  return close(fd);
}

int osremove(const char *name)
{
  return unlink(name);
}

int osmkdir(const char *dir)
{
  return mkdir(dir,0777);
}

int osdup2(int oldfd,int newfd)
{
  return dup2(oldfd,newfd);
}

char *getoserr(ub4 *perrno)
{
  int e = errno;

  if (perrno) *perrno = (ub4)e;
  return e ? strerror(errno) : "";
}

static void wrstderr(const char *buf,ub4 len)
{
  oswrite(2,buf,len);
}

void *osmmapfd(ub8 len,int fd)
{
  int flags = MAP_PRIVATE;
#ifdef MAP_POPULATE
  flags |= MAP_POPULATE;
#endif

  void *p = mmap(NULL,len,PROT_READ,flags,fd,0);
  if (p == MAP_FAILED || globs.sigint) {
    oserror("mmap failed for %lu`B",(ub8)len);
    return NULL;
  }
  return p;
}

void *osmmapfln(ub4 fln,size_t nel,size_t elsiz)
{
  ub8 len;
  void *p;

  if (elsiz >= 65536) fatal(fln,"mmap elsiz %lu exceeds %u",elsiz,65536);
  if (elsiz == 0) fatal(fln,"mmap %lu nil elsiz",nel);
  if (nel >= hi32) fatal(fln,"mmap nel %lu exceeds %u",nel,hi32);
  if (nel == 0) fatal(fln,"mmap %lu nil nel",elsiz);

  len = elsiz * nel;

  if (len >= hi32) fatal(fln,"mmap len %lu exceeds %u",len,hi32);
  if (len < 4096) warnfln(fln,"mmap len %u ln %u",(ub4)len,__LINE__);

#ifdef MAP_ANONYMOUS

  p = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  if (p == MAP_FAILED || (sb8)p < 0) p = nil;

#else
  p = calloc(nel,elsiz);
#endif

  if (p) return p;
  fatal(fln,"mmap failed for %lu * %lu",nel,elsiz);
}

void *osmremapfln(ub4 fln,void *p,size_t elsiz,ub4 oldel,ub4 newel)
{
  ub8 olen = elsiz * oldel;
  ub8 nlen = elsiz * newel;
  void *np;

  if (nlen == olen) {
    warnfln(fln,"os.%u redundant realloc %lu * %u",__LINE__,elsiz,oldel);
    return p;
  } else if (nlen == 0) ice(fln,hi32,"os.%u  realloc to nil %lu * %u",__LINE__,elsiz,oldel);

#ifdef _GNU_SOURCE

  np = mremap(p,olen,nlen,MREMAP_MAYMOVE);
  if (np == MAP_FAILED || (sb8)np < 0) fatal(fln,"os.%u mmremap from %lu`B to %lu`B failed for %p: %m",__LINE__,olen,nlen,p);

#elif defined MAP_ANONYMOUS
  warnfln(fln,"os.%u ignored mremap %lu * %u ln %u",__LINE__,elsiz,oldel);
  return p;
#else

  np = realloc(p,nlen);
  if (np == nil) fatal(fln,"mmremap from %lu` to %lu` failed for %p",olen,nlen,p);
  if (nlen > olen) memset(np + olen,0,nlen - olen);

#endif

  if (nlen < olen && np != p) warning("mmremap from %lu` to %lu` moved %p to %p",olen,nlen,p,np);
  return np;
}

int osmunmapfln(ub4 fln,void *p,size_t len)
{
  int rv = 0;

  if (p == nil) fatal(fln,"nil munmap for len %lu",len);

#ifdef _GNU_SOURCE
  rv = munmap(p,len);
#else
  free(p);
#endif

  return rv;
}

#if 0
int osmemrdonly(void *p,size_t len)
{
  int rv;
  if ((ub8)p & 4095) return error("%p not page-aligned",p);
  rv = mprotect(p,len,PROT_READ);
  return rv;
}
#endif

#if 0

#if (defined(_POSIX_MEMLOCK)) && (_POSIX_MEMLOCK > 0)
int osmlock(void)
{
  if (mlockall(MCL_CURRENT)) { warnfln(FLN,0,"cannot lock memory: %m"); return 1; }
  return 0;
}
int osmunlock(void)
{
  if (munlockall()) { warnfln(FLN,0,"cannot unlock memory: %m"); return 1; }
  return 0;
}
#endif

#endif

static void mysigint(int sig,siginfo_t *si,void *pp)
{
  int n = globs.sigint++;
  char buf[1024];

  eximsg();

  if (globs.sig) _exit(1);

  if (n == 0) msg_errwrite(FLN,globs.shfln,"interrupting: waiting to end current task\n");
  else {
    fmtstring(buf,"interrupted: sig %d code %d%c\n",sig,si->si_code,pp ? ' ' : '.');
    msg_errwrite(FLN,globs.shfln,buf);
   _exit(1);
  }
}

#define Btbuf 32

static Noret void mysigact(int sig,siginfo_t *si,void *pp)
{
  char buf[1024];
  ub4 buflen = 1024;
  ub4 pos;
  void *adr,*nearby;
  int code;
  const char *codestr = "";
  int rv = 1;

  eximsg();

  globs.sig++;

#ifdef USE_GLIBC_EXT
  void *btbuf[Btbuf];
  int btcnt = backtrace(btbuf,Btbuf);

  backtrace_symbols_fd(btbuf,btcnt,2);
#endif

  switch(sig) {
  case SIGSEGV:
    adr = si->si_addr;
    pos = mysnprintf(buf,0,sizeof buf,"\nsigsegv at %p",adr);
    if (adr) {
      nearby = nearblock(adr);
      pos += mysnprintf(buf,pos,sizeof buf," near %p",nearby);
    }
    break;

  case SIGBUS:
    adr = si->si_addr;
    nearby = nearblock(adr);
    pos = mysnprintf(buf,0,sizeof buf,"\nsigbus at %p near %p",adr,nearby);
    break;

  case SIGFPE:
    code = si->si_code;
    switch(code) {
    case FPE_INTDIV: codestr = "int div"; break;
    case FPE_INTOVF: codestr = "int ovf"; break;
    }
    pos = fmtstring(buf,"\nsigfpe errno %d code %d %s",si->si_errno,code,codestr);
    break;

  case SIGILL:
    pos = fmtstring(buf,"\n sigill %u",sig);
    break;

  default:
    pos = fmtstring(buf,"\nsignal %u errno %d", sig,si->si_errno);
  }

  pos += mysnprintf(buf,pos,buflen,"\npid %u\n",globs.pid);
  wrstderr(buf,pos);

  int fd = open(*globs.pidnam ? globs.pidnam : "morelia.pid",O_CREAT|O_WRONLY|O_TRUNC,0644);
  char *p;
  char *q = buf + 16;
  ub4 len;

  if (fd >= 0) {
    *q = '\n';
    p = itoa(q,globs.pid);
    len = (ub4)(q - p + 1);
    if (write(fd,p,len) < 0) { rv = errno; errcnt++; }
    close(fd);
  }

//  pause();
  _exit(rv);
}

void setsigs(void)
{
  struct sigaction sa;

  // make sure libgcc is loaded at handler time
#ifdef USE_GLIBC_EXT
  void *btbuf[4];
  backtrace(btbuf,4);
#endif

  memset(&sa,0,sizeof(sa));

  sa.sa_sigaction = mysigact;

  sa.sa_flags = SA_SIGINFO;

  sigaction(SIGSEGV, &sa,NULL);
  sigaction(SIGFPE, &sa,NULL);
  sigaction(SIGBUS, &sa,NULL);
  sigaction(SIGILL, &sa,NULL);

  sa.sa_sigaction = mysigint;
  sigaction(SIGINT, &sa,NULL);
}

void osmillisleep(ub4 msec)
{
  struct timespec ts;
  ub8 msec2;

  ts.tv_sec = msec / 1000;
  msec2 = (ub8)msec % 1000;
  ts.tv_nsec = (long)(msec2 * 1000UL * 1000UL);

  nanosleep(&ts,NULL);
}

ub8 gettime_usec(void)
{
  struct timespec tv;
  ub8 usec;

#if (defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0)
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tv)) return 0;
#else
  return 0;
#endif

  usec = (ub8)tv.tv_sec * 1000UL * 1000UL;
  usec += (ub8)(tv.tv_nsec / 1000);
  return usec;
}

// physical mem in mb
ub4 osmeminfo(void)
{
#ifdef _SC_PHYS_PAGES

  ub8 pagecnt,mb;
  long lval;

  lval = sysconf(_SC_PHYS_PAGES);
  if (lval < 1) return 0;
  pagecnt = (ub8)lval;
  mb = (ospagesize * pagecnt) >> 20;
  return (ub4)mb;
#else
  return 0;
#endif
}

static void showvmstat(void)
{
#ifdef __linux__
  ub1 buf[4096];
  long len;
  ub8 x=0;
  ub1 *p,*q;

  int fd = open("/proc/self/status",O_RDONLY);

  if (fd == -1) return;

  len = read(fd,buf,4096);
  osclose(fd);

  if (len < 16) return;
  buf[min(len,4095)] = 0;

  p = (ub1 *)strstr((cchar *)buf,"VmPeak:");
  if (p == nil) return;
  q = p + 8;
  while (*q && *q != '\n' && (*q < '0' || *q > '9') ) q++;
  if (*q < '0' || *q > '9') return;
  while (*q >= '0' && *q <= '9') x = x * 10 + *q++ - '0';

  while (*q == ' ') q++;
  if (*q == 'k' || *q == 'K') x *= 1000;
  else if (*q == 'm' || *q == 'M') x *= 1000000;
  else if (*q == 'g' || *q == 'G') x *= 1000000000;

  info("max gross mem use %lu`B",x);
  while (*q && *q != '\n') q++;
  *q = 0;
  vrb("%s",p);
#endif
}

int inios(void)
{
  globs.pid = getpid();

#ifdef _SC_PAGESIZE
  long lval;

  lval = sysconf(_SC_PAGESIZE);
  if (lval < 1) return 0;
  ospagesize = (ub4)lval;
#endif

  return 0;
}

void exios(bool show)
{
  if (errcnt) warning("%u error%.*s",errcnt,errcnt > 1,"s");
  if (show & globs.resusg) showvmstat();
}

#ifdef __clang__
  #pragma clang diagnostic ignored "-Wsign-conversion"
#endif

static int rlimit(int res,rlim_t lim,const char *desc,int show)
{
  struct rlimit rlim;

  if (getrlimit(res,&rlim)) { oserror("cannot get resource limit for %s",desc); return 1; }

  rlim.rlim_cur = min(lim,rlim.rlim_max);
  if (setrlimit(res,&rlim)) { oserror("cannot set resource limit for %s",desc); return 1; }
  genmsg(show ? Info : Vrb,"resource limit for %s set to %lu`",desc,(unsigned long)lim);
  return 0;
}

int oslimits(void)
{
  int rv = 0;
  rlim_t maxvm;

  if (globs.maxvm < hi24) {
    maxvm = (ub8)(globs.maxvm + 4) * 1024 * 1024 * 1024;
    rv |= rlimit(RLIMIT_AS,maxvm,"virtual memory +4 GB margin",1);
  }

  rv |= rlimit(RLIMIT_CORE,0,"core size",0);
  return rv;
}
