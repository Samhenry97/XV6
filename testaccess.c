// a simple test program for the access syscall

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	printf(1, "Testing access call \n");
	/*if(access() == 1)
	{
		printf(1, "Access returned true \n");
	}
	else
	{
		printf(1, "Access did not return true");
	}*/
	printf(1, "No tests at this time");
	exit(0);
}