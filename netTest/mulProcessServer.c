#include<stdio.h>
#include<signal.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

#define SRV_PORT 9999

void strErr(const char*str)
{
    perror(str);
    exit(1);
}

void catchChild(int signum)
{
    while(waitpid(0, NULL, WNOHANG) > 0);
    return;
}

int main(int argc, char*argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
        strErr("socket error");

    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SRV_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(lfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if(ret == -1)
        strErr("bind error");

    ret = listen(lfd, 128);
    if(ret == -1)
        strErr("listen error");

    struct sockaddr_in cltAddr;
    socklen_t cltAddrLen = sizeof(cltAddr);

    int cfd;
    pid_t pid;
    char buf[BUFSIZ];
    while(1)
    {
        cfd = accept(lfd, (struct sockaddr*)&cltAddr, &cltAddrLen);
        pid = fork();
        if(pid < 0)
            strErr("fork error");
        else if(pid == 0)
        {
            close(lfd);
            break;
        }
        else
        {
            struct sigaction act;
            act.sa_flags = 0;
            sigemptyset(&act.sa_mask);
            act.sa_handler = catchChild;

            ret = sigaction(SIGCHLD, &act, NULL);
            if(ret != 0)
                strErr("sigaction error");

            close(cfd);
            continue;
        }
    }

    if(pid == 0)
    {
        for(;;)
        {
            ret = read(cfd, buf, sizeof(buf));
            if(ret == 0)
            {
                close(cfd);
                exit(1);
            }

            for(int i = 0; i < ret; ++i)
                buf[i]= toupper(buf[i]);

            write(cfd, buf, ret);
            write(STDOUT_FILENO, buf, ret);        
        }
    }


    return 0;
}
