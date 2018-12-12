#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  //Declaring an array to hold the name of the program and its arguments.
  char *execargs[argc -1];

  //Doing this so "sudo" doesn't end up being in the arguments list. There may be a better way to do this.
  int i;
  for (i = 1; i < argc; i++) {
    execargs[i-1] = argv[i];
  }
  //just calling exec
  exec(execargs[0], execargs);
  //exec shouldn't return. if we hit this, we fail.
  printf(1, "Exec failed.\n");
  exit(0);

}
