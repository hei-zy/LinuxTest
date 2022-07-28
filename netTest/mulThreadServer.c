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

#define SRV_PORT 9998

struct s_info
{
    struct sockaddr_in cltAddr;
    int cfd;
};

void strErr(const char*str)
{
    perror(str);
    exit(1);
}

void *doWork(void*arg)
{
    struct s_info* ts = (struct s_info*)arg;
    int n;
    char buf[BUFSIZ];
    char str[INET_ADDRSTRLEN];
    while(1)
    {
        n = read(ts->cfd, buf, sizeof(buf));
        if(n == 0)
        {
            printf("the client %d closed...\n", ts->cfd);
            break;
        }
        printf("receiced from %s at PORT %d\n",
                inet_ntop(AF_INET, &(*ts).cltAddr.sin_addr, str, sizeof(str)),
                ntohs(ts->cltAddr.sin_port));

        for(int i = 0; i < n; ++i)
            buf[i] = toupper(buf[i]);

        int ret = write(STDOUT_FILENO, buf, n);
        if(ret == -1)
            strErr("write error");
        ret = write(ts->cfd, buf, n);
    }

    close(ts->cfd);
    return NULL;
}


int main(int argc, char*argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
        strErr("socket error");
    struct sockaddr_in srvAddr;
    bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(SRV_PORT);
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

    int ret = bind(lfd, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
    if(ret == -1)
        strErr("bind error");
    listen(lfd, 128);
    printf("Accepting client connect...\n");

    struct s_info ts[256];
    int i = 0;
    
    struct sockaddr_in cltAddr;
    bzero(&cltAddr, sizeof(cltAddr));
    socklen_t cltAddrLen = sizeof(cltAddr);
    int cfd;
    pthread_t tid;
    while(1)
    {
       cfd = accept(lfd, (struct sockaddr*)&cltAddr, &cltAddrLen);
       if(cfd == -1)
           strErr("accept error");
        ts[i].cltAddr = cltAddr;
        ts[i].cfd = cfd;

       pthread_create(&tid, NULL, doWork, (void*)&ts[i]);
       pthread_detach(tid);
       ++i;
    }


    return 0;
}
