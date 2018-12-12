// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "login", 0 };

int startLogin(char *dev) {
  int pid = fork();
  
  if(pid < 0) {
    printf(1, "init: fork failed\n");
    exit(-1);
  } else if(pid == 0) {
    close(0); close(1); close(2);
    open(dev, O_RDWR); // stdin
    dup(0);            // stdout
    dup(0);            // stderr
    printf(1, "init: starting login for %s\n", dev);
    exec("login", argv);
    printf(1, "init: exec login failed\n");
    exit(-1);
  }
  
  return pid;
}

int
main(void)
{
  int pid1 = -1, pid2 = -1, pid3 = -1, pid4 = -1, wpid;
  int totalDevices = getnumdevices();

  // Device Initialization
  printf(1, "Made it to init\n");
  mknod("console", 1, 0);
  mkdir("/dev");
  if(totalDevices > 0) mknod("/dev/tty0", 1, 0);
  if(totalDevices > 1) mknod("/dev/tty1", 1, 1);
  if(totalDevices > 2) mknod("/dev/tty2", 1, 2);
  if(totalDevices > 3) mknod("/dev/tty3", 1, 3);
  mknod("/dev/null", 5, 1);
  mknod("/dev/zero", 6, 1);
  mknod("/dev/rand", 7, 1);
  mknod("/dev/ide0", 8, 0);
  mknod("/dev/ide1", 8, 1);
  
  open("console", O_RDWR);
  dup(0);
  dup(0);

  while(1) {
    if(totalDevices > 0) { pid1 = startLogin("/dev/tty0"); }
    if(totalDevices > 1) { pid2 = startLogin("/dev/tty1"); }
    if(totalDevices > 2) { pid3 = startLogin("/dev/tty2"); }
    if(totalDevices > 3) { pid4 = startLogin("/dev/tty3"); }
    
    int return_code;
    while(totalDevices > 0 && (wpid=wait(&return_code)) >= 0 && wpid != pid1)
      printf(1, "zombie!\n");
    while(totalDevices > 1 && (wpid=wait(&return_code)) >= 0 && wpid != pid2)
      printf(1, "zombie!\n");
    while(totalDevices > 2 && (wpid=wait(&return_code)) >= 0 && wpid != pid3)
      printf(1, "zombie!\n");
    while(totalDevices > 3 && (wpid=wait(&return_code)) >= 0 && wpid != pid4)
      printf(1, "zombie!\n");
  }
}