#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include<termios.h>
#include<dirent.h>

#define BUFLEN 256

#define ARROWUP 'A'
#define ARROWDOWN 'B'
#define ARROWRIGHT 'C'
#define ARROWLEFT 'D'

#define CTRL_KEY(k) ((k) & 0x1f)

#define NORM_MODE 0
#define INS_MODE 1

#define ESC 27

//lists
struct bufchars{
    struct bufchars *bp;
    struct bufchars *fp;
    char c;
    int len;
};

struct char_line{
    struct char_line *bp, *fp;
    char c;
};

struct lines{
    struct char_line head;
    struct lines *bp, *fp;
    int len;
};
