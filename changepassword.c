#include "user_tools.h"

int write_pass(int uid, char* password){
	int fd = open("users",O_RDWR);
	char buf[80];
	int i = 0;
	while(i < uid){
		read(fd,buf,80);
		i++;
	}

	read(fd,buf,11);
	if(write(fd,password,32) == 32){
		close(fd);
		return 1;
	}

	return 0;
	close(fd);
}

int main(int argc, char const *argv[])
{
	printf(1,"***Change Password***\n");
	//username buffer
	char username[11];

	//password buffer
	char old_pass[35];

	get_username(username);
	int uid = chk_username(username);
	if(uid == -1){
		printf(1,"User not Found!\n");
		exit(1);
	}

	int i = 0;
	while(i < 4){

		printf(1,"***Old Password***\n");
		get_password(old_pass);

		if(check_pass(uid, old_pass) == 0){
			printf(1,"Passwords do not match\n");
			exit(2);
		}else{
			//changepass
			printf(1,"***New Password***\n");
			get_password(old_pass);

			if(write_pass(uid, old_pass) == 1){
				printf(1,"Password Changed!\n");
				exit(0);
			}else{
				printf(1,"ERROR: Password Not Changed!\n");
				exit(-1);
			}
		}
		i++;
	}

	printf(1,"Allowed Attempts Exausted\n");

	exit(3);
}