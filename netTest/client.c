#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<error.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define SERVPORT 9998 

void sys_err(const char*str)
{
    perror(str);
    exit(1);
}

int main(int argc, char*argv[])
{
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVPORT);
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr.s_addr);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
        sys_err("socket error");

    int ret = connect(cfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if(ret != 0)
        sys_err("connect error");

    char buf[BUFSIZ];
    while(1)
    {
        ret = read(STDIN_FILENO, buf, sizeof(buf));
        write(cfd, buf, ret);
        ret = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, ret);
        sleep(1);
    }

    close(cfd);

    return 0;
}
