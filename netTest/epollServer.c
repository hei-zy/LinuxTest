#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<error.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/epoll.h>

#define SERV_PORT 9998

void perrExit(const char*str)
{
   perror(str);
   exit(1);
}


int main(int argc, char*argv[])
{
    // 创建监听套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
        perrExit("socket error");

    // 端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srvAddr;
    bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(SERV_PORT);
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

    int ret = bind(lfd, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
    if(ret == -1)
        perrExit("bind error");

    ret = listen(lfd, 128);
    if(ret == -1)
        perrExit("listen error");
    printf("waiting for client...\n");
    

    char str[INET_ADDRSTRLEN], buf[BUFSIZ];

    int cfd;
    struct sockaddr_in cltAddr;
    bzero(&cltAddr, sizeof(cltAddr));
    socklen_t cltAddrLen = sizeof(cltAddr);

    int efd = epoll_create(FOPEN_MAX);
    if(efd == -1)
        perrExit("epoll_create error");

    struct epoll_event tep, ep[FOPEN_MAX];

    tep.events = EPOLLIN;
    tep.data.fd = lfd;

    int res = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &tep);
    if(res == -1)
        perrExit("epoll_clt error");

    int i;
    int sockfd;
    int n;
    for(;;)
    {
        ret = epoll_wait(efd, ep, FOPEN_MAX, -1);
        if(ret == -1)
            perrExit("epoll_wait error");

        for(i = 0; i < ret; ++i)
        {
            if(!(ep[i].events & EPOLLIN))
                continue;
            
            if(ep[i].data.fd == lfd)
            {
                cfd = accept(lfd, (struct sockaddr*)&cltAddr, &cltAddrLen);
                if(cfd == -1)
                    perrExit("accept error");

                printf("received from %s at PORT %d\n",
                        inet_ntop(AF_INET, &cltAddr.sin_addr, str, sizeof(str)),
                        ntohs(cltAddr.sin_port));
                
                tep.events = EPOLLIN;
                tep.data.fd = cfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &tep);
                if(res == -1)
                    perrExit("epoll_clt error");
            }
            else
            {
                sockfd = ep[i].data.fd;
                n = read(sockfd, buf, sizeof(buf));

                if(n == 0)
                {
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    if(res == -1)
                        perrExit("epoll_ctl error");
                    printf("the client %d closed...\n", sockfd);
                    close(sockfd);
                }
                else if(n < 0)
                {
                    perrExit("read n < 0 error:");

                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    if(res == -1)
                        perrExit("epoll_ctl error");
                    close(sockfd);
                }
                else
                {
                    for(int j = 0; j < n; ++j)
                        buf[j] = toupper(buf[j]);
                    write(sockfd, buf, n);
                    write(STDOUT_FILENO, buf, n);
                }
            }
        }
    }
    close(lfd);

    return 0;
}
