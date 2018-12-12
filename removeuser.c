#include "user_tools.h"
#include "user.h"

int remove_user(int uid){
	char * filename = "users";
	int fd = open(filename,O_RDWR);

	char rd_buff[80];
	char blot[80];
	blot[0] = -1; //uid spot now 255 (-1)
	memset(blot+1,'\0',79);

	int i = 0;
	while(i < uid){
		read(fd,rd_buff,80);
		i++;
	}
	int written = write(fd,blot,80); //uid = -1 and rest 0's
	close(fd);
	if(written != 80){
		printf(1,"User Not Removed: File I/O problem\n");
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[]) {
	//**should be checking for root
	if (0 == getuid()) {
		printf(1,"***Remove User***\n");

		char username[11];
		get_username(username);

		int uid = chk_username(username);

		//check if no user exits or trying to remove root
		if (uid == -1){
			printf(1,"No removeable user found!\n");
			exit(2);
		}else if(uid == 0){
			printf(1,"You won't like what happens if you remove root!\n");
			exit(-1);
		}else{//blot that user out ... make uid -1 and the rest zero out
			if(remove_user(uid) != 0){
				printf(1,"ERROR: User not Removed!\n");
				exit(-1);
			}
		}

		printf(1,"User Removed Successfully!\n");
		print_users();
	}else{
		printf(1,"Must have root access!\n");
		exit(-1);
	}

	exit(0);
}