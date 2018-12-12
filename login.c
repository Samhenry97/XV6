#include "types.h"
#include "user.h"
#include "hash.h"
#include "login.h"
#include "user_tools.h"

void login() {
    if (0 != getuid()) {
        printf(1, "Error: not running as root\n");
        exit(-1);
    }

    printf(2, "***Login***\n");
    printf(2, "Default username / password: 'user' / 'password'\n");
    //password for root: passw0rd
    
    //int fd = open("/users",0);
    //int n, pid;
    char username[9], password[33];
    //char salt[32], hash[32];
    //char entry[80];
    char *argv[] = { "sh", 0 };
    int success = 0, wpid;
    int i=3;
    while(i--) {
        get_username(username);
        int uid = chk_username(username);
        if (uid < 0) {
            printf(1, "Error: invalid username\n");
            continue;
        }

        get_password(password);

        if (check_pass(uid, password) == 0) {
            printf(1, "login failed\n");
            continue;
        } else {
            success = 1;
            int pid = fork();
            if(pid < 0) {
                printf(1, "login: fork for sh failed\n");
                exit(-1);
            } else if(pid == 0) {
                setuid(uid);
                exec("sh", argv);
                printf(1, "login: exec sh failed\n");
                exit(-1);
            }
            int return_code;
            while((wpid = wait(&return_code)) >= 0 && wpid != pid)
                ; // Do nothing
            
            break;
        }
    }
    if(!success) { printf(1, "3 failed login attempts, restarting...\n"); }
}

int main(int argc, char *argv[]) {
    while(1) { login();  }
    exit(0);
}
