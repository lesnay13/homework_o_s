#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>

#define port 8088

int main(int argc, char* argv[])
{
    int fds[2];
    char tab[BUFSIZ];
    int fd, n;

    char *fifo1 = "/tmp/server";
    char *fifo2 = "/tmp/client";

    pipe(fds);
    mkfifo(fifo1,0666);

    while(1)
    {
        fds[0]=open(fifo2,O_RDONLY);
        fds[1]=open(fifo1,O_WRONLY);

        read(fds[0],tab,BUFSIZ);

        if (strcmp("Client",tab)==0) {
            printf("Client Connection: %s\n",tab);
            fd=open(tab,O_WRONLY);

            if(fork()==0)
            {
                dup2(fds[1],1);
                close(fds[1]);
                execlp("ls","ls","-l",NULL);
                close(fds[0]);
                close(fds[1]);
            }
            else
            {
                dup2(fds[0],0);
                n = read(fds[0],tab,BUFSIZ);
                write(fd,tab,n);
                close(fds[0]);
                close(fds[1]);
            }
        }
        memset(tab, 0, sizeof(tab));
        close(fd);
        close(fds[0]);
        close(fds[1]);
    }

    unlink(fifo1);
    return 0;
}