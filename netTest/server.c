#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<unistd.h>
#include<signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<error.h>
#include<ctype.h>

#define SERVPORT 9998 



void sysErr(const char*str)
{
    perror(str);
    exit(1);
}


int main(int argc, char* argv[])
{
    struct sockaddr_in servAddr;


    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVPORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
        sysErr("socket error");

    bind(lfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    listen(lfd, 128);

    char remote[INET_ADDRSTRLEN];
    printf("server ip: %s and port: %d\n", inet_ntop(AF_INET, &servAddr.sin_addr, remote, INET_ADDRSTRLEN), ntohs(servAddr.sin_port));

    struct sockaddr_in clitAddr; 
    bzero(&clitAddr, sizeof(clitAddr));
    socklen_t clitAddrLength = sizeof(clitAddr);
    int cfd= accept(lfd, (struct sockaddr*)&clitAddr, &clitAddrLength);
    if(cfd == -1)
        sysErr("accept error");

    char buf[1024];
    int ret;
    while(1)
    {
        ret = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, ret);

        for(int i = 0; i < ret; ++i)
        {
            buf[i] = toupper(buf[i]);
        }
        write(cfd, buf,ret);
    }

    close(lfd);
    close(cfd);

    return 0;
}
