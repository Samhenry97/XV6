#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{  
  if(argc != 3) {
    printf(1, "Usage: ./color foreground [0-7] background [0-7]");
    exit(1);
  }
  
  int fg = atoi(argv[1]) + 30;
  int bg = atoi(argv[2]) + 40;
  
  printf(1, "\33[%d;%dm", fg, bg);
  
  exit(0);
}
