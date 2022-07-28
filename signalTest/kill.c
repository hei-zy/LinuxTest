#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>


int main(int argc, char*argv[])
{
    int i;

    pid_t *cpid = mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(cpid == MAP_FAILED)
    {
        perror("mmap error");
        exit(1);
    }

    for(i = 0; i < 5; ++i)
    {
        if(fork() == 0)
            break;
    }

    if(i == 3)
    {
        *cpid = getpid();
        while(1);
    }
    if(i == 5)
    {
        sleep(1);
        printf("kill the %d\n", *cpid);
        kill(*cpid, SIGKILL);
    }

    return 0;
}
