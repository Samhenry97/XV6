#include "types.h"
#include "stat.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "buf.h"


/*
README:
	This has been tested by running cat /dev/id0 > /dev/ide1 and then cat /dev/ide1 > /dev/ide0.
	It has a seek function to set the position in the ide, but for testing there is commented out code that allows the following to run without doing seeks:
		cat /dev/ide0 > /dev/ide1
		cat /dev/ide1 > /dev/ide0 // this first attempt to write ide1 to ide0 will fail, but it should reset the position so that the second attempt succeeds
		cat /dev/ide1 > /dev/ide0
	There are also commented out prints in the functions to monitor progress. Copying the drives back and forth can take a bit of time. 
*/

int ideAddress[2];

int alignedNumber(int address) {
	if ((address % BSIZE) != 0) {
		// if the address doesn't evenly divide by the block size, round to the nearest block
		unsigned int remaining = address % BSIZE;
		unsigned int numberWholeBlocks = address / BSIZE;
		address = ((remaining) < (BSIZE / 2)) ? (address - remaining) : ((numberWholeBlocks * BSIZE) + BSIZE);
	}
	return address;
}

int idedriverwrite(struct inode *ip, char *buf, int n) {
	iunlock(ip);
	
	unsigned int blockAddress = ideAddress[ip->minor];
	unsigned int blockNo = blockAddress / BSIZE;

	// make sure they aren't trying to write past end of fs
	if ((blockNo >= (FSSIZE - 1)) || blockAddress >= (BSIZE * FSSIZE)) {
		// cprintf(ALL_DEVS, "Trying to write past end of FS. BlockNo: %d blockAddress: %d", blockNo, blockAddress);
		ideAddress[ip->minor] = 0;
		ilock(ip);
		return 0;
	}

	struct buf *buffer = bread(ip->minor, blockNo);

	// make sure n is 0 mod BSIZE
	unsigned int numberBytes = alignedNumber(n);
	for (int i = 0; i < numberBytes; ++i) {
		int relativeIndex = i % BSIZE;
		buffer->data[relativeIndex] = buf[relativeIndex];

		int nextBlockPastFSEnd = (((blockAddress + BSIZE) / BSIZE) >= FSSIZE) ? 1 : 0;
		int finishedReadingBlock = ((i + 1) % BSIZE) == 0;

		// make sure you haven't hit the end of the file system
		if (finishedReadingBlock && !nextBlockPastFSEnd) { 
			// move to next block
			bwrite(buffer);
			brelse(buffer);
			blockAddress += BSIZE;
			blockNo = blockAddress / BSIZE;
			buffer = bread(ip->minor, blockNo);

			// cprintf(ALL_DEVS, "Write: next block: %d address: %d\n", blockNo, blockAddress);
		}
		else if (finishedReadingBlock && nextBlockPastFSEnd) {
			// Uncomment for testing -----------------------------------------------------------------------------------------------------------
			// cprintf(ALL_DEVS, "Hit end of FS at writing to address: %d and block number: %d\n", blockAddress, blockNo);
			// numberBytes = i;
			// ideAddress[ip->minor] = 0;	// loop back around to beginning for testing
			// bwrite(buffer);
			// brelse(buffer);
			// ilock(ip);
			// return numberBytes;

			// make sure to return how many bytes you've read before hitting the end of the file system
			numberBytes = i;
			break;
		}
	}

	bwrite(buffer);
	brelse(buffer); 

	// store position 
	ideAddress[ip->minor] = blockAddress; 
	ilock(ip);

	return numberBytes;
}

int idedriverread(struct inode *ip, char *dst, int n) {
	iunlock(ip);

	unsigned int blockAddress = ideAddress[ip->minor];
	unsigned int blockNo = blockAddress / BSIZE;

	if ((blockNo >= (FSSIZE - 1)) || blockAddress >= (BSIZE * FSSIZE)) {
		// trying to read past end of fs
		// cprintf(ALL_DEVS, "Trying to read past end of FS. BlockNo: %d blockAddress: %d", blockNo, blockAddress);
		ideAddress[ip->minor] = 0;
		ilock(ip);
		return 0;
	}

	// cprintf(ALL_DEVS, "Read %d blocks starting at block address %d\n", n / BSIZE, blockAddress);

	struct buf *buffer = bread(ip->minor, blockNo);

	// make sure n is 0 mod BSIZE 
	unsigned int numberBytes = alignedNumber(n);
	for (int i = 0; i < numberBytes; ++i) {
		int relativeIndex = i % BSIZE;
		dst[relativeIndex] = buffer->data[relativeIndex];
		
		int nextBlockPastFSEnd = (((blockAddress + BSIZE) / BSIZE) >= FSSIZE) ? 1 : 0;
		int finishedReadingBlock = ((i + 1) % BSIZE) == 0;

		// make sure you haven't hit the end of the file system
		if (finishedReadingBlock && !nextBlockPastFSEnd) { 
			// move to next block
			brelse(buffer);
			blockAddress += BSIZE;
			blockNo = blockAddress / BSIZE;
			buffer = bread(ip->minor, blockNo);
			// cprintf(ALL_DEVS, "Read: next block: %d address: %d\n", blockNo, blockAddress);
		}
		else if (finishedReadingBlock && nextBlockPastFSEnd) {
			// Uncomment for testing -----------------------------------------------------------------------------------------------------------
			// cprintf(ALL_DEVS, "Hit end of FS at writing to address: %d and block number: %d\n", blockAddress, blockNo);
			// numberBytes = i;
			// ideAddress[ip->minor] = 0;	// loop back around to beginning for testing
			// brelse(buffer);
			// ilock(ip);
			// return numberBytes;

			// make sure to return how many bytes you've read before hitting the end of the file system
			numberBytes = i;
			break;
		}
	}

	brelse(buffer);

	// store current position
	ideAddress[ip->minor] = blockAddress;
	ilock(ip);

	return numberBytes;
}

int idedriverseek(struct inode *ip, int address) {
	iunlock(ip);
	unsigned int blockAddress = alignedNumber(address);
	// cprintf(ALL_DEVS, "Address given: %d block sought to: %d\n", address, blockAddress);
	ideAddress[ip->minor] = blockAddress;
	ilock(ip);
	return 0;
}

void idedriverinit(void) {
	devsw[IDE].read = idedriverread;
	devsw[IDE].write = idedriverwrite;
	devsw[IDE].seek = idedriverseek;

	ideAddress[0] = 0;
	ideAddress[1] = 0;
}