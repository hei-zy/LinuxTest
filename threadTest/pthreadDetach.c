#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<string.h>
#include<pthread.h>


void* threadCall(void*arg)
{
    printf("thread call\n");
    return NULL;
}

int main(int argc, char*argv[])
{
    pthread_t thd;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thd, &attr, threadCall, (void*)NULL);
    pthread_attr_destroy(&attr);


    void* retval;
    int err;
    while(1)
    {
        sleep(1);
        err = pthread_join(thd, &retval);
        if(err != 0)
            fprintf(stderr, "thread_join error: %s\n", strerror(err));
        else
            fprintf(stderr, "thread exit code %d\n", *(int*)retval);
    }


    return 0;
}
