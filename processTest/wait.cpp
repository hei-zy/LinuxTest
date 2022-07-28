#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cstring>
#include<cerrno>
#include<sys/wait.h>


int main(int argc,char*argv[])
{
    pid_t pid = fork();
    int status;
    
    if(pid == 0)
    {
        printf("---child, my id= %d,going to sleep 10s\n", getpid());
        sleep(10);
        printf("----child die------\n");
    }
    else if(pid > 0)
    {
        pid_t wpid = wait(&status);    
        if(wpid == -1)
        {
            perror("wait error\n");
            exit(1);
        }
        if(WIFEXITED(status))
        {
            printf("child exit with %d\n", WEXITSTATUS(status));
        }
        if(WIFSIGNALED(status))
        {
            printf("child kill with signal %d\n", WTERMSIG(status));
        }

        printf("-------parent wait finish: %d\n", wpid);
    }
    else
    {
        perror("fork error\n");
        exit(1);
    }

    return 0;
}
