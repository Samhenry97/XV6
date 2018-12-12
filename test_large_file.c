#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

int
main(int argc, char *argv[])
{

  	int fd;
	fd = open("max_test.txt", O_CREATE | O_RDWR);
	if(fd < 0){
		printf(1, "Open failed\n");
		exit(1);
	}

	//Allocate enough space for one disk block
	char buf[512];

	//Use dev/rand to get junk to fill test file
	int ran = open("/dev/rand", O_RDONLY);
	if(ran < 0){
		printf(1, "Open failed.\n");
		exit(1);
	}

	int rand_blocks[5];

	printf(1, "Generating random block list.\n");
	for(int i = 0; i < 5; ++i){
		read(ran, buf, 1);
		rand_blocks[i] = buf[0] % MAXFILE;
	}

	char rand_buffers[5][5];

	//Used to compare what was written to with what was read
	// char rand_block_buf1[5];
	char file_block[5];

	// char rand_block_buf2[5];

	// char rand_block_buf3[5];

	// char rand_block_buf4[5];

	// char rand_block_buf5[5];

	//Write 281 blocks to the file
	//This is the maximum number of blocks allowable per file
	//26 Direct Blocks + 127 Indirect blocks + 128 Indirect blocks
	printf(1, "Generating random file of max size: %d blocks.\n", MAXFILE);
	for(int i = 0; i < 281; i++){
		if(read(ran, buf, 512) < 0){
			printf(1, "Read failed.\n");
			exit(1);
		}
		for(int j = 0; j < 5; j++){
			if(i == rand_blocks[j]){
				memmove(rand_buffers[j], buf, 5);
				break;
			}
		}
		// if(i == rand_block_1){
		// 	memmove(rand_block_buf1, buf, 5);
		// }
		// else if(i == rand_block_buf2){
		// 	memmove(rand_block2, buf, 5);
		// }
		// else if(i == rand_block_3){
		// 	memmove(rand_block_buf_3, buf, 5);
		// }
		// else if(i == rand_block_4){
		// 	memmove(rand_block_buf_4, buf, 5);
		// }
		// else if(i == rand_block_5){
		// 	memmove(rand_block_buf_5, buf, 5);
		// }
		if(write(fd, buf, 512) < 0){
			printf(1, "Write failed on write %d\n", i);
			exit(1);
		}
	}

	close(ran);
	char * fmt_string = "Beginning of block %d from %s: %d %d %d %d %d\n";

	for(int i = 0; i < 5; i++){
		printf(1, fmt_string, rand_blocks[i], "rand", rand_buffers[i][0], rand_buffers[i][1], rand_buffers[i][2], rand_buffers[i][3], rand_buffers[i][4]);
		seek(fd, rand_blocks[i]*512, SEEK_SET);
		read(fd, file_block, 5);
		printf(1, fmt_string, rand_blocks[i], "file", file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);
	}

	// printf(1, fmt_string, rand_block_1, rand_block_buf1[0], rand_block_buf1[1], rand_block_buf1[2], rand_block_buf1[3], rand_block_buf1[4]);
	// seek(fd, rand_block_1*512, SEEK_SET);
	// read(fd, file_block, 5);
	// printf(1, fmt_string, rand_block_1, file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);

	// printf(1, fmt_string, rand_block_1, rand_block_buf1[0], rand_block_buf1[1], rand_block_buf1[2], rand_block_buf1[3], rand_block_buf1[4]);
	// seek(fd, rand_block_1*512, SEEK_SET);
	// read(fd, file_block, 5);
	// printf(1, fmt_string, rand_block_1, file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);

	// printf(1, fmt_string, rand_block_1, rand_block_buf1[0], rand_block_buf1[1], rand_block_buf1[2], rand_block_buf1[3], rand_block_buf1[4]);
	// seek(fd, rand_block_1*512, SEEK_SET);
	// read(fd, file_block, 5);
	// printf(1, fmt_string, rand_block_1, file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);

	// printf(1, fmt_string, rand_block_1, rand_block_buf1[0], rand_block_buf1[1], rand_block_buf1[2], rand_block_buf1[3], rand_block_buf1[4]);
	// seek(fd, rand_block_1*512, SEEK_SET);
	// read(fd, file_block, 5);
	// printf(1, fmt_string, rand_block_1, file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);

	// printf(1, fmt_string, rand_block_1, rand_block_buf1[0], rand_block_buf1[1], rand_block_buf1[2], rand_block_buf1[3], rand_block_buf1[4]);
	// seek(fd, rand_block_1*512, SEEK_SET);
	// read(fd, file_block, 5);
	// printf(1, fmt_string, rand_block_1, file_block[0], file_block[1], file_block[2], file_block[3], file_block[4]);

	close(fd);
	if(unlink("max_test.txt") < 0){
		printf(1, "Unlink file failed.\n");
	}
	exit(0);
}
