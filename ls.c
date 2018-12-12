#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

char * permissions(struct stat * st){
  char * perm = "----------";
  memset(perm, '-', 10);
  if(st->type == T_DIR){
    perm[0] = 'd';
  }
  if((st->permissions & 256) == 256){
    perm[1] = 'r';
  }
  if((st->permissions & 128) == 128){
    perm[2] = 'w';
  }
  if((st->permissions & 64) == 64){
    perm[3] = 'x';
  }
  if((st->permissions & 32) == 32){
    perm[4] = 'r';
  }
  if((st->permissions & 16) == 16){
    perm[5] = 'w';
  }
  if((st->permissions & 8) == 8){
    perm[6] = 'x';
  }
  if((st->permissions & 4) == 4){
    perm[7] = 'r';
  }
  if((st->permissions & 2) == 2){
    perm[8] = 'w';
  }
  if((st->permissions & 1) == 1){
    perm[9] = 'x';
  }

  return perm;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if (access(path, O_LSDIR) != 1){
    printf(2, "ls: permission denied for %s\n", path);
    return;
  }

  printf(1, "In ls\n");

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf(1, "%s %d %s %d %d %d\n", permissions(&st), st.owner, fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      printf(1, "%s %d %s %d %d %d\n", permissions(&st), st.owner, fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(-1);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
