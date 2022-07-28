#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>


int main(int argc,char*argv[])
{
    pid_t pid = fork();
    if(pid == -1)
    {
        perror("fork error\n");
        exit(1);
    }
    else if(pid == 0)
    {
        execlp("ls","ls","-l","-a","-i",NULL);
    }
    else if(pid > 0)
    {
        sleep(1);
        printf("I'm parent : %d\n", getpid());
    }

    return 0;
}
