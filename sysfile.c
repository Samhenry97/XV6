//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

#define PATH_MAX 256
static char cwd[PATH_MAX] = "/";
static char PATH[PATH_MAX] = "/";

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=proc->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;

  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd] == 0){
      proc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  proc->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

int sys_chmod(void){
  struct inode *ip;
  char * path;
  int perms;
  if(argstr(0, &path) < 0){
    return -1;
  }

  if(argint(1, &perms) < 0){
    return -1;
  }
  if((ip = namei(path)) == 0) {
    return -1;
  }

  begin_op();
  ilock(ip);

  if(proc->uid != ip->owner){
    iunlock(ip);
    end_op();
    return -1;
  }

  
  ip->permissions = perms;
  iupdate(ip);
  iunlock(ip);
  end_op();

  return 1;
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  int user = proc->uid;
  int dir_perms = dp->permissions;
  int dir_owner = dp->owner;
  int own_write = ((user == dir_owner) && ((dir_perms & 128) == 128));
  int global_write = ((user != dir_owner) && ((dir_perms & 2) == 2));

  if(!own_write && !global_write && user != 0){
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  // cprintf("Unlinked %s.\n", name);

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  uint off;
  struct inode *ip, *dp;
  char name[DIRSIZ];
  //NEED TO CHANGE TO GETUID but it doesnt exist right now
  uint current_user = proc->uid;
  //Default permissions for directory: rwxr-xr-x (755 octal)(493 decimal)
  //Default permissions for file: rw-rw-rw- (666 octal) (438 decimal)
  uint default_permissions_directory = 493;
  uint default_permissions_file = 438;

  if((dp = nameiparent(path, name)) == 0){
    return 0;
  }
  ilock(dp);

  if((ip = dirlookup(dp, name, &off)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  ip->owner = current_user;
  if(type == T_DIR){
    ip->permissions = default_permissions_directory;
  }
  else{
    ip->permissions = default_permissions_file;
  }
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);
  // if(ip != 0 && !holdingsleep(&ip->lock))
  //   iunlock(ip);
  return ip;
}


char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int access_path_check(char *);

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;


  if(argstr(0, &path) < 0 || argint(1, &omode) < 0 || !access_path_check(path))
    return -1;

  struct inode *pip;
  char name[DIRSIZ];

  if((pip = nameiparent(path, name)) == 0){
    return -1;
  }
  ilock(pip);
  int dir_perms = pip->permissions;
  //Check whether owner
  //ADD GETUID;
  int user = proc->uid;
  int dir_owner = pip->owner;
  iunlockput(pip);
  int own_write = ((user == dir_owner) && ((dir_perms & 128) == 128));
  int global_write = ((user != dir_owner) && ((dir_perms & 2) == 2));

  //Creating Bitmask of of omode
  int n_omode = 0;
  if((omode & O_RDWR) == O_RDWR){
    n_omode |= 6;
  }
  else if(omode == O_RDONLY){
    n_omode |= 4;
  }
  else if(((omode & O_WRONLY) == O_WRONLY) || ((omode & O_TRUNC) == O_TRUNC)){
    n_omode |= 2;
  }

  begin_op();

  if((omode & O_CREATE) && (user == 0 || own_write || global_write)){
    ip = create(path, T_FILE, 0, 0);
    
    if(ip == 0){
      end_op();
      return -1;
    }


    if(ip->owner == user){
      n_omode = n_omode << 6;
    }
    if((ip->permissions & n_omode) != n_omode){
      iunlock(ip);
      end_op();
      return -1;
    }


  } else {
    if(access_things(path, n_omode) >= 0){
      if((ip = namei(path)) == 0){
        end_op();
        return -1;
      }
      ilock(ip);
      if(ip->type == T_DIR && omode != O_RDONLY){
        iunlockput(ip);
        end_op();
        return -1;
      }
    }
    else{
      end_op();
      return -1;
    }
  }


  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }

  if (omode & O_TRUNC){
    itrunc(ip);
  }
  
  iunlock(ip);
  end_op();


  if (omode & O_APPEND){
    f->off = ip->size - 1;
  } else{
    f->off = 0;
  }

  f->type = FD_INODE;
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;
  char tmp[PATH_MAX+256];

  if(argstr(0, &path) < 0){
    return -1;
  }

  begin_op();


  //Crahes something dealing with links and mkdir
  //Dont know what yet
  // struct inode *pip;
  // char name[DIRSIZ];

  // char n_path[512];
  // strcpy(n_path, path);
  // // cprintf("%s\n", n_path);

  // char * cur_path = skipelem(n_path, name);
  // while(*cur_path != '\0' && cur_path != 0) {
  //   // cprintf("%s\n", n_path);
  //   cur_path = skipelem(cur_path, name);
  // }

  // if((pip = nameiparent(path, name)) == 0){
  //   // panic("mkdir: Problem");
  //   return -1;
  // }
  // ilock(pip);
  // int dir_perms = pip->permissions;
  // //Check whether owner
  // //ADD GETUID;
  // int user = 0;
  // int dir_owner = pip->owner;
  // iunlockput(pip);
  // int own_write = ((user == dir_owner) && ((dir_perms & 128) == 128));
  // int global_write = ((user != dir_owner) && ((dir_perms & 2) == 2));

  // //check for ability to read in directory
  // if((dir_owner == user) && (!own_write)) {
  //   end_op();
  //   return -1;
  // }
  // else if(dir_owner != user && (!global_write)){
  //   end_op();
  //   return -1;
  // }

  int r = argstr(0, &path);
  if (path[0] != '/') { //if not absolute path, concat cwd to path
    strncpy(tmp, cwd, PATH_MAX);
    strncat(tmp, path, PATH_MAX);
    ip = create(tmp, T_DIR, 0, 0);
  } else {
    ip = create(path, T_DIR, 0, 0);
  }
  if(r < 0 || ip == 0){
    end_op();
    return -1;
  }

  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_ioctl(void) {
  struct file *f;
  int key, value;
  
  if(argfd(0, 0, &f) < 0 || argint(1, &key) < 0 || argint(2, &value) < 0) {
    return -1;
  }
  
  devsw[f->ip->major].ioctl(f->ip, key, value); // Call ioctl() on the device
  return 0;
}

int sys_getnumdevices(void) {
  return uartgetnumdevices();
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

//Obatined from http://stackoverflow.com/questions/20342559/how-to-cut-part-of-a-string-in-c
/*
 *      Remove given section from string. Negative len means remove
 *      everything up to the end.
 */
int str_cut(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}

int access_path_check(char * path){
  char * path_div;
  if(*path == '/'){
    path++;
    path_div = strchr(path, '/');
    path--;
  }
  else{
    path_div = strchr(path, '/');
  }
  while(path_div != 0){
    char working_copy[512];
    strcpy(working_copy, path);
    str_cut(working_copy,  strlen(path) - strlen(path_div), strlen(path_div));
    if(access_things(working_copy, 1) < 0){
      return 0;
    }
    path_div++;
    path_div = strchr(path_div, '/');
  }

  return 1;
}

void update_cwd(char*);


int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if (argstr(0, &path) < 0 || path[0] == '\0'){ // no arg given? go to root
    path[0] = '/';
    path[1] = '\0';
  }
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  

  if(!access_path_check(path) || access_things(path, 1) <= 0){
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }


  iunlock(ip);
  iput(proc->cwd);

  update_cwd(path);

  end_op();
  proc->cwd = ip;
  return 0;
}

void update_cwd(char* path) {
  // first off, if there is a trailing forward slash, get rid of it
  int i;
  int len = PATH_MAX;
  for (i = 0; i < PATH_MAX; ++i) {
    if (path[i] == '\0') {
      len = i;
      break;
    }
  }
  if (path[len-1] == '/' && len > 1) path[len-1] = '\0';

  // now update cwd, taking each part of the path (even if only one) one at a time
  char* pathWhole = '\0';
  safestrcpy(pathWhole, path, PATH_MAX);
  int done = 1;

  do {
    done = 1;
    // get a part of the cd'd path
    int i;
    for (i = 1; i < PATH_MAX && pathWhole[i] != '\0'; ++i) {
      if (pathWhole[i] == '/') { // split it!
        done = 0;
        pathWhole[i] = '\0';
        safestrcpy(path, pathWhole, PATH_MAX);
        safestrcpy(pathWhole, &pathWhole[i+1], PATH_MAX);
        break;
      }
    }

    if (done) { // we didn't do any more splits, so evaluate the rest of the path
      safestrcpy(path, pathWhole, PATH_MAX);
    }

    // check each possible case for this part of the path
    if (path[0] == '/') { // is it an absolute path?
      safestrcpy(cwd, path, PATH_MAX);
    } else if (path[0] == '.' && path[1] == '.') { // is it back a directory?
      int lastIndex = PATH_MAX;
      int secondToLastIndex = PATH_MAX;
      int i = 0;
      for (; i < PATH_MAX && cwd[i] != '\0'; ++i) {
        if (cwd[i] == '/') {
          secondToLastIndex = lastIndex;
          lastIndex = i;
        }
      }
      if (secondToLastIndex != PATH_MAX) { // if I have found a "second to last foward slash"
        cwd[secondToLastIndex] = '\0'; // NUL-term at the index of the second to last forward slash
      }
    } else if (path [0] == '.') { // am I staying in the current directory?
      // do nothing
    } else { // it's a relative path, so just cat it to the end
      strncat(cwd, path, PATH_MAX);
    }

    // add a foward slash to the end if there is not one there already
    len = PATH_MAX;
    for (i = 0; i < PATH_MAX; ++i) {
      if (cwd[i] == '\0') {
        len = i;
        break;
      }
    }
    if (cwd[len-1] != '/') strncat(cwd, "/\0", PATH_MAX);

  } while (!done);
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(PATH, path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      proc->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

// returns the value of the cwd in the buffer that is passed in
int
sys_getcwd(void) {
  char* arg[PATH_MAX];
  argstr(0, arg); // add error checking
  safestrcpy(arg[0], cwd, PATH_MAX);

  return 0;
}

int
sys_seek(void){
  int fd, offset, ref_pos;
  struct file *f;

  if(argfd(0, &fd, &f) < 0 || argint(1, &offset) < 0 || argint(2, &ref_pos) < 0)
    return -1;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return -1;
  if(f->type == FD_INODE){

    int new_pos = 0;
    switch(ref_pos){
      case SEEK_SET:
        new_pos = offset;
        break;
      case SEEK_CUR:
        new_pos = f->off + offset;
        break;
      case SEEK_END:
        new_pos = f->ip->size - 1 - offset;
        break;
    }

    if (new_pos < 0 || new_pos >= f->ip->size){
      return -1;
    }

    ilock(f->ip);
    f->off = new_pos;
    iunlock(f->ip);
    return new_pos;
  }
  panic("seek");
}


int
sys_mount(void) { 
  char *mount;
  char *path;
  char *mpt = "/mnt\0";
  if(argstr(0, &mount) < 0 || argstr(1, &path) < 0)
    return -1;

  if(strncmp(mount, mpt, 7*sizeof(char))!=0){
    cprintf(1, "unable to mount %s != %s\n", mount, mpt);
    return -1;
  }
  begin_op();
  struct inode *ip = namei(mount);
  if(ip == 0){
    ip = create(mount, T_DIR, 0, 0);
    //ilock(ip);
    ip->mount = 'Y';
    cprintf(ALL_DEVS, "Drive %s succesfully mounted\n", mount);
    iunlockput(ip);
    end_op();
    return 0;
  } else {
    //ilock(ip);
    if(ip->mount == 'N') {
      ip->mount = 'Y';
      cprintf(ALL_DEVS, "Drive %s succesfully mounted\n", mount);
      
      end_op();
      return 0;
    } else {
      cprintf(ALL_DEVS, "Drive %s is already mounted\n", mount);
      
      end_op();
      return 0;
    }
  }
  end_op();
  return -1;
}
