#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char* argv[])
{
    int fds[2];
    char *fifo = "/tmp/server";
    char *fifo2 = "/tmp/client";

    mkfifo(fifo2,0666);
    fds[0]=open(fifo,O_RDONLY);
    fds[1]=open(fifo2,O_WRONLY);

    char tab[BUFSIZ];
    memset(tab, 0, sizeof(tab));

    write(fds[1],"Client",6);
    perror("Write:"); //Very crude error check
    read(fds[0],tab,sizeof(tab));
    perror("Read:"); // Very crude error check

    printf("Message from server: %s\n",tab);

    close(fds[0]);
    close(fds[1]);
    unlink(fifo2);
    return 0;
}