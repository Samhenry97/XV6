#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

//place holders till hash.h works
void getSalt(char* salt);
void hashPassword(char *password, char* salt, char* hashed_pass);

int chk_username(char * user);
void get_username(char * username);
void get_password(char * password);
void print_users();
int check_pass(int uid, char * password);
void clearPassword(char* password);