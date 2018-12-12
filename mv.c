#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

void mv(char* source, char* dest) {
	// link(newFilename, oldFilname)
	if (link(source, dest) == 0) {
		unlink(source);
	} else {
		printf(1, "Could not move file: %s to %s.\n", source, dest);
	}
}

int main(int argc, char*argv[]) {
	// check source and destination files specified
	if (argc < 3) {
		printf(1, "usage: mv <source> <destiniation>\n");
		exit(-1);
	}
	mv(argv[1], argv[2]);
	exit(0);
}