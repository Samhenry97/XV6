#include "types.h"
#include "stat.h"
#include "user.h"
#include "types.h"

int
main(int argc, char *argv[]) {
  if(argc < 3){
    printf(2, "Usage: mount /mnt ???\n");
    exit(1);
  }
  int i = mount(argv[1], argv[2]);
  if(i < 0){
    printf(1, "invalid inputs\n");
  }
  exit(0);
}
