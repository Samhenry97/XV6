#include "types.h"
#include "fcntl.h"
#include "user.h"

int
main(int argc, char **argv)
{
    int fin = -1, fout = -1;
    int count = -1;
    char buff[64];

    if (argc < 3) {
        printf(2, "Error: must have IN and OUT file\n");
        exit(-1);
    }
    
    if (argc == 4) {
        count = atoi(argv[3]);
    }
    if ((fin = open(argv[1], O_RDONLY)) < 0) {
        printf(2, "Error: can't open '%s'\n", argv[1]);
        exit(-2);
    }
    if ((fout = open(argv[2], O_CREATE | O_WRONLY)) < 0) {
        printf(2, "Error: can't open '%s'\n", argv[2]);
        exit(-2);
    }

    if (count < 0) {
        int sz;
        while ((sz = read(fin, buff, sizeof buff)) > 0) {
            if (write(fout, buff, sz) < sz) {
                printf(2, "Error: couldn't write [all] data to output\n");
                exit(-3);
            }
        }
        if (sz < 0) {
            printf(2, "Error: couldn't read data from input\n");
            exit(-4);
        }
    }
    else {
        int sz;
        while (count > 0) {
            sz = read(fin, buff, (count > sizeof buff) ? sizeof buff : count);
            if (sz < 0) {
                printf(2, "Error: couldn't read data from input\n");
                exit(-4);
            }
            if (write(fout, buff, sz) < sz) {
                printf(2, "Error: couldn't write [all] data to output\n");
                exit(-3);
            }
            count -= sz;
        }
    }
    exit(0);
}

