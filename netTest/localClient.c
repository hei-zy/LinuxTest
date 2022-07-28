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
#define CLIT_ADDR "clit.socket"


int main(int argc ,char *argv[])
{
    char buf[BUFSIZ];

    int cfd = socket(AF_UNIX, SOCK_STREAM, 0);



    struct sockaddr_un clitAddr;
    bzero(&clitAddr, sizeof(clitAddr));
    clitAddr.sun_family = AF_UNIX;
    strcpy(clitAddr.sun_path, CLIT_ADDR);
    socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(clitAddr.sun_path);

    unlink(CLIT_ADDR);
    bind(cfd, (struct sockaddr*)&clitAddr, len);

    struct sockaddr_un servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sun_family = AF_UNIX;
    strcpy(servAddr.sun_path, SERV_ADDR);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servAddr.sun_path);

    connect(cfd, (struct sockaddr*)&servAddr, len);

    while(fgets(buf, sizeof(buf), stdin) != NULL)
    {
        write(cfd, buf, strlen(buf));
        len = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);
    }

    close(cfd);


    return 0;
}
