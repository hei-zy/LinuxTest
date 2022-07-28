#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<error.h>


void sigCatch(int sig)
{
    if(sig == SIGINT)
    {
        printf("catch the signal %d\n", sig);
        sleep(10);
    }   
    else if(sig == SIGTSTP)
    {
        printf("catch the signal %d\n", sig);
    }
}

int main(int argc, char*argv[])
{
    struct sigaction act, oldact;
    act.sa_handler = sigCatch;    
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGQUIT);
    act.sa_flags = 0;
    
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    int ret = sigaction(SIGINT, &act, &oldact);
    if(ret == -1)
    {
        perror("sigaction error");
        exit(1);
    }
    ret = sigaction(SIGTSTP, &act, &oldact);
    if(ret == -1)
    {
        perror("sigaction error");
        exit(1);
    }

    sigprocmask(SIG_UNBLOCK, &set, NULL);
    while(1);


    return 0;
}
