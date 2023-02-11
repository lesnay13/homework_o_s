/*
 * FIFOcommunication.c
 *
 *  Created on: Feb 2, 2023
 *      Author: user1
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#define MAXLINE 4096



/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char    buf[MAXLINE];
    vsnprintf(buf, MAXLINE, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",
    strerror(error));
    strcat(buf, "\n");
    fflush(stdout);     /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL);       /* flushes all stdio output streams */
}


/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */

void
err_sys(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}

void client(int readfd, int writefd) {

}

void server(int readfd, int writefd) {

}
int main () {
    int readfd, writefd;
    pid_t childpid;

    char* FIFO1 = "/tmp/FIFO1";
    char* FIFO2 = "/tmp/FIFO2";
    int FILE_MODE = 777; //read, write, and execute permissions

    if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
        err_sys("Can't create %s", FIFO1);
    if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
        unlink(FIFO1);
        err_sys("Can't create %s", FIFO2);
    }

    if ((childpid = fork()) == 0) {
        readfd = open(FIFO1, O_RDONLY, 0);
        writefd = open(FIFO2, O_WRONLY, 0);
        server(readfd, writefd);
        exit(0);
    }

    writefd = open(FIFO1, O_WRONLY, 0);
    readfd = open(FIFO2, O_RDONLY, 0);
    client(readfd, writefd);
    waitpid(childpid, NULL, 0);
    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);


    return 0;
}