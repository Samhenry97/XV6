#include "hash.h"
//#include "user_tools.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

//security measure so that the password doesn't stay in memory
void clearPassword(char* password) {
	int i = 35;
	while (i--) {
		password[i] = 0;
	}
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

int main() {
    char username[8];
    username[0] = 'r';
    username[1] = 'o';
    username[2] = 'o';
    username[3] = 't';
    username[4] = ' ';
    username[5] = ' ';
    username[6] = ' ';
    username[7] = ' ';
    
    char password[32];
    clearPassword(password);
    strcopy(password, "passw0rd", 8);
    
    char salt[32];
    char hash[32];
    
    getSalt(salt);
    hashPassword(password, salt, hash);
    
    write_user(0, 0, username, hash, salt);
    
    username[0] = 'u';
    username[1] = 's';
    username[2] = 'e';
    username[3] = 'r';
    username[4] = ' ';
    username[5] = ' ';
    username[6] = ' ';
    username[7] = ' ';

    clearPassword(password);
    strcopy(password, "password", 8);

    getSalt(salt);
    hashPassword(password, salt, hash);
    
    write_user(1, 1, username, hash, salt);
}