#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<error.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>

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


    int cfd;
    struct sockaddr_in cltAddr;
    bzero(&cltAddr, sizeof(cltAddr));
    socklen_t cltAddrLen = sizeof(cltAddr);

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(lfd, &allset);

    int i;
    char str[INET_ADDRSTRLEN], buf[BUFSIZ];
    int client[FD_SETSIZE];
    for(i = 0; i < FD_SETSIZE; ++i)
        client[i] = -1;

    int maxfd = lfd;
    int maxi = -1;
    
    int sockfd;
    int n;
    while(1)
    {
        rset = allset;
        ret = select(maxfd+1, &rset, NULL, NULL, NULL);
        if(ret < 0)
            perrExit("select error");

        if(FD_ISSET(lfd, &rset))
        {
            cfd = accept(lfd, (struct sockaddr*)&cltAddr, &cltAddrLen);
            if(cfd == -1)
                perrExit("accept error");
            printf("received from %s at PORT %d\n",
                    inet_ntop(AF_INET, &cltAddr.sin_addr, str, sizeof(str)),
                    ntohs(cltAddr.sin_port));
            
            for(i = 0; i < FD_SETSIZE; ++i)
            {
                if(client[i] < 0)
                {
                    client[i] = cfd;
                    break;
                }
            }
            if(i == FD_SETSIZE)
            {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            FD_SET(cfd, &allset);
            
            if(maxfd < cfd)
                maxfd = cfd;
            if(maxi < i)
                maxi = i;

            if(ret == 1)
                continue;
        }

        for(i = 0; i <= maxi; ++i)
        {
            if((sockfd = client[i]) < 0)
                continue;
            if(FD_ISSET(sockfd, &rset))
            {
                n = read(sockfd, buf, sizeof(buf));
                if(n == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else if(n > 0)
                {
                    for(int j = 0; j < n; ++j)
                        buf[j] = toupper(buf[j]);
                    write(sockfd, buf, n);
                    write(STDOUT_FILENO, buf, n);
                }
                if(ret == 1)
                    break;
            }
        }
    }
    close(lfd);


    return 0;
}
