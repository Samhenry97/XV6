#include "user_tools.h"
#include "hash.h"
#include "user.h"

//assumes users stored in order of uid
int nextUID() {
	char * filename = "users";
	int fd = open(filename,2);

	char uid;
	char junk[80];

	int max = 0;
	int bytes_read = -1;

	while(bytes_read != 0){
		read(fd,&uid,1);
		
		if (uid >= max) {
			max = uid + 1;
		}

		bytes_read = read(fd,junk,79);
	}

	close(fd);
	return max;
}

int write_user(int _uid, int _gid, char* username, char* password, char* salt) {
	char * c = ",";
	char* end = "\n\0";

	char uid[2];
	uid[0] =  _uid;

	//default group id for now 1
	char gid[2];
	gid[0] = gid[0] + _uid;

	//file i/0
	char * filename = "users";
	int fd = open(filename,O_RDWR);

	int written = 0;
	char rd_buff[80];
	int i = 0;

	while(i < _uid){
		read(fd,rd_buff,80);
		i++;
	}

	written += write(fd,uid,1);
	written += write(fd,c,1);
	written += write(fd,username,8);
	written += write(fd,c,1);
	written += write(fd,password,32);
	written += write(fd,c,1);

	written += write(fd,salt,32);
	written += write(fd,c,1);

	written += write(fd,gid,1);
	written += write(fd,end,2);
	close(fd);

	if (written != 80){ return 1; }

	return 0;
}


int main(int argc, char const *argv[]) {
	printf(1,"***Add User***\n");
	//username buffer
	char username[11];
	//password buffer
	char password[35];

	//get user ID verify root

	//if root***
	if(0 == getuid()){
		get_username(username);

		//checks duplicate username
		if(chk_username(username) != -1){
			printf(1,"Username is already taken!\n");
			exit(2);
		}

		get_password(password);

		//find next UID
		int uid = nextUID();

		//set group id???
		int gid = 1;

		//hash password
		char salt[32];
		char hashed_pass[32];

		getSalt(salt);
		hashPassword(password, salt, hashed_pass);

		if (write_user(uid,gid,username,hashed_pass,salt) != 0) {
			printf(1,"ERROR: User not Added!\n");
		} else {
			printf(2,"User Added!\n");
		}
	} else {
		printf(1,"Only root can use Add User\n");
		exit(1);
	}
	clearPassword(password);
	print_users();
	exit(0);
}