// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

#define NDIRECT 26
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + (NINDIRECT -1) + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint owner;			// User ID of file owner
  //Bitfield explanation
  //Note: integers are 32 bits long
  //Current Structure: 000000000000000000000(padding) 0(directory field) 000(Normal user permissions)0(S bit) 000(Group permissions) 000(global permissions)
  //00000000000000000000 000 (S-bits) 000 (User) 000 (Group) 000 (Other)
  //Total bits used: 12
  //Maybe used ushort?
  uint permissions;		// Bitfield used for file permissions
  // char alignment[56];	// Alignment field to pad dinode out to 128 bytes. System assumes blocksize is evenly divisible by the indoe size. Should probably figure out why.
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};
