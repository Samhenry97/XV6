#include "user_tools.h"
#include "hash.h"

//security measure so that the password doesn't stay in memory
void clearPassword(char* password) {
    int i = 35;
    while (i--) {
        password[i] = 0;
    }
}

//buffer of length > 10
void get_username(char * username){
    int i = 11;
    // initialize the password to all zeros
    while (i--) {
        username[i] = ' ';
    }
    //get username
    printf(1,"Please Enter Username: ");
    gets(username, 9);
    int changing = 0;
    for (i = 0; i < 8; i++) {
        if (username[i] == '\n') {
            changing = 1;
        }
        if (changing) {
            username[i] = ' ';
        }
    }
    username[8] = 0;
}

//buffer of length > 34
void get_password(char * password){
    clearPassword(password);
    //get password
    printf(1,"Please Enter Password: ");
    
    ioctl(1, IOCTL_MASK, 1);    // Mask input
    gets(password, 33);
    ioctl(1, IOCTL_MASK, 0);

    int i;
    int changing = 0;
    for (i = 0; i < 32; i++) {
        if (password[i] == '\n') {
            changing = 1;
        }
        if (changing) {
            password[i] = 0;
        }
    }

    password[32] = 0;
}

//returns 0 if password does not match
//returns 1 if password matches
int check_pass(int uid, char* password){
    char * filename = "users";
    int fd = open(filename,2);

    char buffer[80];
    char junk[12];
    char hashword[33];
    hashword[32] = '\0';

    char salt[32];

    int i = 0;
    while(i < uid){
        read(fd,buffer,80);
        i++;
    }
    read(fd,junk,11);
    read(fd,hashword,32);
    read(fd,junk,1);
    read(fd,salt,32);
    close(fd);
    //hash the attempted password
    char h_pass[33];
    h_pass[32] = '\0';
    
    hashPassword(password, salt, h_pass);
    if(strcmp(hashword,h_pass) != 0){
        return 0;
    }

    return 1;

}

//returns -1 if username not found;
//returns uid of user if
int chk_username(char * new_name){
    char username[9];
    char prejunk[3];
    char postjunk[70];

    char * filename = "users";
    int fd = open(filename,2);

    int bytes_read = -1;
    username[8] = 0;
    while (bytes_read != 0) {
        read(fd,prejunk,2);
        read(fd,username,8);
        int i;
        for (i = 0; i < 8; i ++) {
            //printf(2,"%d:%d;\n",username[i],new_name[i]);
        }
        
        if (strcmp(username, new_name) == 0) {
            close(fd);
            return prejunk[0];
        }

        bytes_read = read(fd,postjunk,70);
    }

    close(fd);
    return -1;
}

void print_users(){
    char * filename = "users";
    int fd = open(filename,2);
    
    //with this code here, the whole project won't compile
    /*if(fd < 0){
        printf(2, "Users open failed.\n");
        exit(1);
    }*/

    //char uid[2];
    char uid;
    char gid[2];
    gid[1] = '\0';
    char username[8];
    char password[32];
    char salt[32];

    char trash[8];

    int bytes_read = -1;
    while(bytes_read != 0){
        bytes_read = read(fd,&uid,1);
        if (bytes_read == 0) {
            break;
        }
        read(fd,trash,1);
        read(fd,username,8);
        read(fd,trash,1);
        read(fd,password,32);
        read(fd,trash,1);
        read(fd,salt,32);
        read(fd,trash,1);
        read(fd,gid,1);
        bytes_read = read(fd,trash,2);
        if (uid != -1) {
            printf(1,"UID: %d\n", uid);
            printf(1,"USER: %s\n", username);
        }
    }
}