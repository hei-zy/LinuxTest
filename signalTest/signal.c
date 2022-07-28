#include<signal.h>
#include<stdio.h>

void sigCatch(int sig)
{
    printf("catch the %d\n", sig);
}


int main(int argc,char* argv[])
{
    signal(SIGINT, sigCatch);

    while(1);


    return 0;
}
