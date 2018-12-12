#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

#include "date.h"
#include "fcntl.h"
#include "elf.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;

  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)proc->sz;
  for(s = *pp; s < ep; s++)
    if(*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint(proc->tf->esp + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;

  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= proc->sz || (uint)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_access(void);
extern int sys_chmod(void);
extern int sys_getuid(void);
extern int sys_setuid(void);
extern int sys_getppid(void);
extern int sys_getcwd(void);
extern int sys_seek(void);
extern int sys_mount(void);
extern int sys_ioctl(void);
extern int sys_getnumdevices(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_access]  sys_access,
[SYS_chmod]   sys_chmod,
[SYS_getuid]  sys_getuid,
[SYS_setuid]  sys_setuid,
[SYS_getppid] sys_getppid,
[SYS_getcwd]  sys_getcwd,
[SYS_seek]    sys_seek,
[SYS_mount]   sys_mount,
[SYS_ioctl]   sys_ioctl,
[SYS_getnumdevices] sys_getnumdevices
};

void
syscall(void)
{
  int num;

  num = proc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    proc->tf->eax = syscalls[num]();
  } else {
    cprintf(ALL_DEVS, "%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}

// return whether or not access is allowed
int
access_things(char* path, int omode)
{
  // int hasAccess = 0;
  struct inode *ip;
  int user = proc->uid;

  // if(argstr(0, &path) < 0 || argint(1, &omode) < 0) { return -1; }

  // check user (currently just root/0)
  // int user = getuid();

  // if user is root, always grant access
  if(user == 0) { return 1; }

  if((ip = namei(path)) == 0) { return -1; }
  ilock(ip);
  // cprintf("%d %d %d %d\n", st.ino, st.permissions, ip->inum, ip->permissions);
  if(ip->owner == user){
    int n_omode = omode << 6;
    // int res = (ip->permissions & n_omode);
    // int b = ((ip->permissions & n_omode) != n_omode);
    // cprintf("%d %d\n", ip->permissions, n_omode);

    if((ip->permissions & n_omode) != n_omode){
      // cprintf("Access failed\n");
      iunlockput(ip);
      return -1;
    }
  }
  else{
    // cprintf("%d %d %d\n", omode & ip->permissions, omode, user);
    if((ip->permissions & omode) != omode){
      // cprintf("Access failed\n");
      iunlockput(ip);
      return -1;
    }
  }
  // if(ip->owner == user) { hasAccess = 1; }
  // else if(ip->permissions == omode) { hasAccess = 1; }
  iunlockput(ip);
  return 1;
}
