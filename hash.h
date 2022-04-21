/* hash.h - hash functions

  MurmurHash 3, placed in the public domain by Austin Appleby
 */

#ifdef __clang__
  __attribute__((no_sanitize("unsigned-integer-overflow")) )
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wcast-align"
#endif

// murmur3 https://github.com/aappleby/smhasher  last changed 9 Jan 2016
static inline unsigned int scramble(unsigned int k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

// Hash 4-byte aliged string
static unsigned int hashalstr(const unsigned char *s,unsigned int len,unsigned int seed)
{
	unsigned int h = seed;
  unsigned int i,k,m,len4 = len >> 2;
  const unsigned int *s4;

  if (len4) {
    s4 = (const unsigned int *)s;
    s += len4 << 2;
    i = 0;
    do {
      k = s4[i++];
      s += 4;
      h ^= scramble(k);
      h = (h << 13) | (h >> 19);
      h = h * 5 + 0xe6546b64;
    } while (i < len4);
  }

  m = 0;
  switch(len & 3) {
  case 3: m = s[2] << 16; Fallthrough;
  case 2: m ^= s[1] << 8; Fallthrough;
  case 1: m ^= s[0]; Fallthrough;
  case 0:
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
  }
  return h;
}

#ifdef __clang__
  #pragma clang diagnostic pop
#endif

/* Fowler-Noll-Vo aka FNV 1a hash http://www.isthe.com/chongo/tech/comp/fnv
   placed in the public domain by Creative Commons CC0 1.0 universal license
 */

#define prim64 0x00000100000001B3UL
#define ofs64 0xcbf29ce484222325UL

static ub8 hash64fnv(cchar *s,ub4 len,ub8 h)
{
  do {
    h ^= *s++;
    h *= prim64;
  } while (--len);
  return h;
}

static ub8 hash64(cchar *s,ub4 len)
{
  return hash64fnv(s,len,ofs64);
}
