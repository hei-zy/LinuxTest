#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<sys/signal.h>
#include<sys/wait.h>
#include<signal.h>


void sysError(const char *str)
{
    perror(str);
    exit(1);
}

void catchChild(int sig)
{
    pid_t wpid;
    int status;

    while((wpid = waitpid(-1, &status, 0)) != -1)
    {
        if(WIFEXITED(status))
        {
            printf("--------catch child id %d, ret = %d\n", wpid, WEXITSTATUS(status));
        }
    }
}


int main(int argc,char* argv[])
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL); // set blockMask

    int i;
    for(i = 0; i < 15; ++i)
    {
        if(fork() == 0)
            break;
    }

    if(i == 15)
    {
        struct sigaction act;
        act.sa_flags = 0;
        sigemptyset(&(act.sa_mask));
        act.sa_handler = catchChild;
        sigaction(SIGCHLD, &act, NULL);
        
        sigprocmask(SIG_UNBLOCK, &set, NULL); // unset blockMask

        printf("I am parent, pid = %d\n", getpid());
        while(1);
    }
    else
    {
        printf("I am child, pid = %d\n", getpid());
        return i;
    }

    return 0;
}
