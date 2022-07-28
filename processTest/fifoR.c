#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>

int main(int argc, char*argv[])
{
    char buf[1024];
    if(argc < 2)
    {
        printf("./a.out fifoname\n");
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if(fd < 0)
    {
        printf("open error\n");
        exit(1);
    }

   
    int size;
    while(1)
    {
        size = read(fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, size);
        sleep(1);
    }
    close(fd);



    return 0;
}

