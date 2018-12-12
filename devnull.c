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

int devnullwrite(struct inode *ip, char *buf, int n) {
  return n; // We "wrote" n characters
}

int devnullread(struct inode *ip, char *dst, int n) {
  return 0; // EOF
}

void devnullinit(void) {
  devsw[DEVNULL].write = devnullwrite;
  devsw[DEVNULL].read = devnullread;
}
