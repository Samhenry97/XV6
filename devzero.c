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

int devzerowrite(struct inode *ip, char *buf, int n) {
  return n;
}

int devzeroread(struct inode *ip, char *dst, int n) {
  int i;
  
  iunlock(ip);
  
  for(i = 0; i < n; i++) {
    *dst++ = '0';
  }
  
  ilock(ip);
  
  return n;
}

void devzeroinit(void) {
  devsw[DEVZERO].write = devzerowrite;
  devsw[DEVZERO].read = devzeroread;
}
