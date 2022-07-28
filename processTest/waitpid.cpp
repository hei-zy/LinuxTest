#include<stdio.h>
#include<stdlib.h>
#include<cerrno>
#include<cstring>
#include<unistd.h>
#include<sys/wait.h>


int main(int argc,char*argv[])
{
    int i;
    for(i = 0; i < 5; ++i)
    {
        if(fork() == 0)
            break;
    }
    if(i == 5)
    {
        pid_t wpid;
        while((wpid = waitpid(-1, NULL, WNOHANG)) != -1)
        {
            if(wpid > 0)
                printf("wait child %d\n", wpid);
            else if(wpid == 0)
            {
                sleep(1);
                continue;
            }
        }
    }
    else
    {
        sleep(i);
        printf("I'm %dth child, pid= %d\n", i+1, getpid());
    }


    return 0;
}
