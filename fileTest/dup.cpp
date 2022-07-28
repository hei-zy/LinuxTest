#include<stdio.h>
#include<unistd.h>
#include<cstring>
#include<fcntl.h>
#include<stdlib.h>

int main(int argc, char*argv[])
{
    int fd1 = open(argv[1], O_RDWR);
    int fd2 = open(argv[2], O_RDWR);

    int fdret = dup2(fd1, fd2);
    
    printf("fd1= %d\n", fd1);
    printf("fd2 = %d\n", fd2);
    printf("fdret = %d\n", fdret);

    int ret = write(fd2,"123456", 7);
    printf("ret = %d\n", ret);

    dup2(fd2, STDOUT_FILENO);
    printf("-----------------------123\n");

    close(fd1);
    return 0;
}
