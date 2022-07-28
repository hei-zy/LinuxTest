#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<sys/wait.h>


int main(int argc,char*argv[])
{
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1)
    {
        perror("pipe error\n");
        exit(1);
    }

    pid_t pid;
    int i;
    for(i = 0; i < 2; ++i)
    {
        pid = fork();
        if(pid == 0)
            break;
    }

    if(i == 2)
    {
        close(fd[0]);
        close(fd[1]);
        while(waitpid(-1, NULL, 0) != -1)
        {
            sleep(1);
        }
    }
    if(i == 0)
    {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
        perror("wc error\n");
        exit(1);
    }
    if(i == 1)
    {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execlp("ls", "ls", NULL);
        perror("ls error\n");
        exit(1);
    }

    return 0;
}
