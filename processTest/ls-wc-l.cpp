#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cstring>
#include<cerrno>



int main(int argc,char*argv[])
{
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1)
    {
        perror("pipe error\n");
        exit(1);
    }

    pid_t pid = fork();
    if(pid > 0)
    {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
        perror("wc error\n");
        exit(1);
    }
    else if(pid == 0)
    {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execlp("ls", "ls", NULL);
        perror("ls error\n");
        exit(1);
    }
    else
    {
        perror("fork error\n");
        exit(1);
    }


    return 0;
}
