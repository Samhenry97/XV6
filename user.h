struct stat;
struct rtcdate;

#define IOCTL_CLEAR 7
#define IOCTL_MASK 42

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int access(char*, int);
int chmod(char *, int);
int getuid(void);
int setuid(int);
int getppid(void);
int getcwd(char*);
int seek(int, long int, int);
int mount(char*, char*);
int ioctl(int, int, int);
int getnumdevices(void);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* fgets(int fd, char*, int max);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int changemode(char *, int);
char* strncat(char *dest, const char *src, int n);