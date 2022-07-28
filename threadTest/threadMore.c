#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<error.h>



void* threadcall(void* arg)
{
    int i = (int)arg;
    printf("this is %dth thread, pid= %d, threadID= %lu\n", i, getpid(), pthread_self());
    
    return NULL;
}


int main(int argc,char*argv[])
{
    printf("pid= %d, threadID= %lu\n", getpid(), pthread_self());
    
    pthread_t thread;
    int ret;
    for(int i = 0; i < 5; ++i)
    {
        ret = pthread_create(&thread, NULL, threadcall, (void*)i);
        if(ret == -1)
        {
            perror("pthread_create error\n");
            exit(1);
        }
    }

    sleep(1);
    printf("this is the main thread\n");




    return 0;
}
