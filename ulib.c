#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

//basically the same as gets used to 
//be but now allows for a unique <fd>
//as a parameter so that we can read
//from any file
char*
fgets(int fd, char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(fd, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

//does the same thing as it used to
//but is now redirected through fgets()
//to allow for a different file to be 
//read besides stdin
char*
gets(char* buf, int max)
{
  fgets(0, buf, max);
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  // printf(1, "%s\n", n);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}


//Used to change permissions of a file by name
int changemode(char *n, int permissions){

  //Call chmod
  if(chmod(n, permissions) < 0){
    return -1;
  }
  return 1;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}
