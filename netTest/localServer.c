#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<stddef.h>



#define SERV_ADDR "serv.socket"


int main(int argc, char*argv[])
{
    char buf[BUFSIZ];
    
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un servAddr;

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sun_family = AF_UNIX;
    strcpy(servAddr.sun_path, SERV_ADDR);

    socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(servAddr.sun_path);

    unlink(SERV_ADDR);
    bind(lfd, (struct sockaddr*)&servAddr, len);

    listen(lfd, 20);
    printf("Accept...\n");


    struct sockaddr_un clitAddr;
    int n;
    int cfd;
    while(1)
    {
        len = sizeof(clitAddr);
        cfd = accept(lfd, (struct sockaddr*)&clitAddr, &len);

        len += offsetof(struct sockaddr_un, sun_path);

        clitAddr.sun_path[len] = '\0';
        printf("client bind filename %s\n", clitAddr.sun_path);

        while((n = read(cfd ,buf, sizeof(buf))) > 0)
        {
            for(int i = 0; i < n; ++i)
            {
                buf[i] = toupper(buf[i]);
            }
            write(cfd, buf, n);
        }
        close(cfd);
    }
    close(lfd);

    return 0;
}
