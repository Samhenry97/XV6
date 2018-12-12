#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  if(argc != 3) {
    printf(1, "Usage: ./cursor [direction amount] [x y]");
    exit(1);
  }
  
  int amt = atoi(argv[2]);
  if(strcmp(argv[1], "up") == 0) {
    printf(1, "\33[%dA", amt);
  } else if(strcmp(argv[1], "down") == 0) {
    printf(1, "\33[%dB", amt);
  } else if(strcmp(argv[1], "right") == 0) {
    printf(1, "\33[%dC", amt);
  } else if(strcmp(argv[1], "left") == 0) {
    printf(1, "\33[%dD", amt);
  } else {
    int x = atoi(argv[1]);
    int y = amt;
    
    printf(1, "\33[%d;%dH", x, y);
  }
  
  exit(0);
}
