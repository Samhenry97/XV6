#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

void
seektest(int fd)
{
  int n;
  // int s = seek(fd, 30, SEEK_CUR);  // update for testing as needed

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      printf(1, "seektest: write error\n");
      exit(1);
    }
  }
  if(n < 0){
    printf(1, "seektest: read error\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    exit(1);
  }

  for(i = 1; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "seektest: cannot open %s\n", argv[i]);
      exit(1);
    }
    seektest(fd);
    close(fd);
  }
  exit(0);
}
