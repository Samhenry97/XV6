#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int pid = fork();
  
  if (pid == 0) {
      //child
      exit(1337);
  } else {
      int returnCode;
      int pid2 = wait(&returnCode);
      if (pid2 == pid && returnCode == 1337) {
          printf(1, "Return codes work!!!\n");
      } else if (pid2 == pid) {
          printf(1, "The return code is wrong!!! %d\n", returnCode);
      } else if (returnCode == 1337) {
          printf(1, "The PID is wrong!!!\n");
      } else {
          printf(1, "Return codes don't work?!?!?\n");
          exit(1); //we're still going to return a return code that doesn't work :)
      }
  }
  
  exit(0);
}
