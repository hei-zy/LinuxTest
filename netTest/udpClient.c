#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<ctype.h>


#define SERV_PORT 8000

int main(int argc, char*argv[])
{
    char buf[BUFSIZ];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    int n;
    while(1)
    {
        n = read(STDIN_FILENO, buf, sizeof(buf));
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(n == -1)
            perror("sendto error");

        n = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, 0);
        if(n == -1)
            perror("recvfrom error");

        write(STDOUT_FILENO, buf, n);
    }

    close(sockfd);


    return 0;
}
