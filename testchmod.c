#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int
main(int argc, char *argv[])
{
  	int fd;
	fd = open("a.txt", O_CREATE);
	close(fd);
	struct stat st;
	stat("a.txt", &st);
	printf(1, "%d\n", st.permissions);
	changemode("a.txt", 959);
	stat("a.txt", &st);
	printf(1, "%d\n", st.permissions);
	unlink("a.txt");
	exit(0);
}
