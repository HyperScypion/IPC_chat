#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXLINE 1024

struct my_msgbuf {
    long mtype;
    char mtext[MAXLINE];
    char nickname[MAXLINE];
    long pid;
    int option;
};
