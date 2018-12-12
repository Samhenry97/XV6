#include "types.h"
#include "stat.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// https://en.wikipedia.org/wiki/Mersenne_Twister
// Define MT19937 constants (32-bit RNG)
enum {
    // Assumes W = 32 (omitting this)
    N = 624,
    M = 397,
    R = 31,
    A = 0x9908B0DF,
    F = 1812433253,
    U = 11,
    S = 7,
    B = 0x9D2C5680,
    T = 15,
    C = 0xEFC60000,
    L = 18,
    MASK_LOWER = (1ull << R) - 1,
    MASK_UPPER = (1ull << R)
};

static unsigned int  mt[N];
static unsigned int  index;

static void init(const int seed) {
  int i;
  
  mt[0] = seed;
  for(i = 1; i < N; i++) {
    mt[i] = (F * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i);
  }
  
  index = N;
}

static void twist() {
  unsigned int  i, x, xA;

  for ( i = 0; i < N; i++ )
  {
      x = (mt[i] & MASK_UPPER) + (mt[(i + 1) % N] & MASK_LOWER);

      xA = x >> 1;

      if ( x & 0x1 )
          xA ^= A;

      mt[i] = mt[(i + M) % N] ^ xA;
  }

  index = 0;
}

static unsigned int rand() {
  unsigned int y;
  int i = index;
  
  if(index >= N) {
    twist();
    i = index;
  }
  
  y = mt[i];
  index++;
  
  y ^= (mt[i] >> U);
  y ^= (y << S) & B;
  y ^= (y << T) & C;
  y ^= (y >> L);

  return y;
}

int devrandwrite(struct inode *ip, char *buf, int n) {
  return n;
}

int devrandread(struct inode *ip, char *dst, int n) {
  int i;
  
  iunlock(ip);
  
  for(i = 0; i < n; i++) {
    *dst++ = (char) (rand() % 255);
  }
  
  ilock(ip);
  
  return n;
}

void devrandinit(void) {
  devsw[DEVRAND].write = devrandwrite;
  devsw[DEVRAND].read = devrandread;
  
  init(sys_uptime());
}
