#include "types.h"
#include "stat.h"
#include "user.h" 
#include "fcntl.h"

int
main(int argc, char *argv[])
{

  int num_devices = getnumdevices();

  for (int i = 0; i < num_devices; ++i) {
    char dev_name[10] = "/dev/ttya";
    dev_name[8] = '0' + i;

    int dev = open(dev_name, O_RDWR);

    for (int j = 1; j < argc; ++j)
      printf(dev, "%s%s", argv[j], j+1 < argc ? " " : "\n");
    
    close(dev);
  }

  exit(0);
}
