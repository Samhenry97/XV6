#include "types.h"
#include "stat.h"
#include "user.h"

int otoi(char *octal) {
    int value = 0;

    for (; *octal != '\0'; ++octal) {
        char digit = *octal;
        if ('0' <= digit && digit <= '7') {
            value = (value << 3) + (digit - '0');
        } else {
            return 0;
        }
    }
    return value;
}


int
main(int argc, char *argv[])
{
    int i;

    uint perms;
    perms = otoi(argv[1]);
    for(i = 2; i < argc; i++){
        if(changemode(argv[i], perms) < 0){
            printf(1, "chmod failed.\n");
        }
    }
    exit(0);
}
