#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int result;
  printf(2, "uid: %d\n", getuid());
  result = setuid(2);
  printf(2, "result: %d, uid: %d\n", result, getuid());
  result = setuid(0);
  printf(2, "result: %d, uid: %d\n", result, getuid());
  exit(0);
}