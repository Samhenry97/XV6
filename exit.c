#include "types.h"
#include "user.h"
#include "stat.h"

int main() {
    //kill the shell
    kill(getppid());
    //kill myself
    exit(0);
}