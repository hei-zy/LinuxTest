#include<stdio.h>
#include<stdlib.h>
#include<cerrno>
#include<unistd.h>
#include<cstring>


int main(int argc, char*argv[])
{
    printf("before fork-1-\n");

    int i;
    for(int i=0; i < 5;++i)
    {
        if(fork() == 0)
        {
            printf("I'am %dth child\n", i);
            break;
        }
    }

    if(i == 5)
    {
        sleep(1);
        printf("I'm parent\n");

    }


    return 0;
}
