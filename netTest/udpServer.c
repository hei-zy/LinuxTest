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
    char str[INET_ADDRSTRLEN];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serv_addr, clie_addr;
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    printf("Accepting connections...\n");
    
    socklen_t clie_addr_len = sizeof(clie_addr);
    
    int n;
    while(1)
    {
        
        n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&clie_addr, &clie_addr_len);
        if(n == -1)
            perror("recvfrom error");

        printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),
                ntohs(clie_addr.sin_port));

        printf("%s\n", buf);

        for(int i = 0; i < n; ++i)
            buf[i] = toupper(buf[i]);

        n = sendto(sockfd, buf, n, 0, (struct sockaddr*)&clie_addr, clie_addr_len);
        if(n == -1)
            perror("sendto error");
    }

    close(sockfd);


    return 0;
}
