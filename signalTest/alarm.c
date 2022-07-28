#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<error.h>
#include<sys/time.h>
#include<signal.h>

void print(int signo)
{
    printf("hello world\n");
}

int main(int argc,char* argv[])
{
   struct itimerval it,oldit;

   signal(SIGALRM, print);

   it.it_value.tv_sec = 2;
   it.it_value.tv_usec = 0;

   it.it_interval.tv_sec = 5;
   it.it_interval.tv_usec = 0;

   if(setitimer(ITIMER_REAL, &it, &oldit) == -1)
   {
        perror("setitimer error");
        return -1;
   }
   
   while(1);

    return 0;
}
