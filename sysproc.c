#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "fcntl.h"
#include "elf.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  int code;
  if(argint(0, &code) < 0)
    return -1;
  exit(code);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int* return_code;
  if(argptr(0, (char**) &return_code, sizeof(int)) < 0)
    return -1;
  return wait(return_code);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// return whether or not access is allowed
int
sys_access(void)
{
  // int hasAccess = 0;
  // struct inode *ip;
  // int user = 0;
  int omode;
  char * path;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0) { return -1; }

  // // check user (currently just root/0)
  // // int user = getuid();

  // if((ip = namei(path)) == 0) { return -1; }
  // if(ip->owner == user) { hasAccess = 1; }
  // else if((ip->permissions & omode) == omode) { hasAccess = 1; }

  // return hasAccess;

  return access_things(path, omode);
}

int
sys_getuid(void){
  return proc->uid;
}

int
sys_setuid(void)
{
  int id;

  if(argint(0,&id) < 0){
    return -1;
  }
  
  if(proc->uid == 0 && id >= 0){
    proc->uid = id;
    return id;  
  }else{
    return -1;
  }
}

int sys_getppid(void) {
  return proc->parent->pid;
}
