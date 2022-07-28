#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<sys/stat.h>
#include<fcntl.h>


int main(int argc,char*argv[])
{
    pid_t pid = fork();
    if(pid > 0)
        exit(1);
    else if(pid < 0)
    {
        perror("fork error");
        exit(1);
    }
    
    pid = setsid();
    if(pid == -1)
    {
        perror("setsid error");
        exit(1);
    }

    int ret = chdir("/home/zy");
    if(ret == -1)
    {
        perror("chdir error");
        exit(1);
    }

    umask(0022);

    close(STDIN_FILENO);
    
    int fd = open("/dev/null", O_RDWR);
    if(fd == -1)
    {
        perror("open error");
        exit(1);
    }

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);


    while(1);   //
    


    return 0;
}
