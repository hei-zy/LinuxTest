#include<stdio.h>
#include<signal.h>
#include<stdlib.h>



void printSet(sigset_t *set)
{
   for(int i = 1; i < 32; ++i)
   {
        if(sigismember(set, i) == 1)
            putchar('1');
        else
            putchar('0');
   }
   printf("\n");
}

int main(int argc, char* argv[])
{
    sigset_t set, pedset;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    int ret = sigprocmask(SIG_BLOCK, &set, NULL);
    if(ret == -1)
    {
        perror("sigprocmask error");
        exit(1);
    }


    while(1)
    {
        ret = sigpending(&pedset);
        if(ret == -1)
        {
            perror("sigpending error");
            exit(1);
        }
        printSet(&pedset);
    }



    return 0;
}
