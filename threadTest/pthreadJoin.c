#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<pthread.h>
#include<string.h>

typedef struct threadData
{
    int var;
    char str[256];
}thrd;

void* threadCall(void*arg)
{
    thrd* retval = (thrd*)arg;
    retval->var = 111;
    strcpy(retval->str, "hello thread");

    return (void*)retval;
}



int main(int argc, char*argv[])
{
    pthread_t thd;
    thrd arg;
    thrd *retval;
    int ret = pthread_create(&thd, NULL, threadCall, (void*)&arg);
    if(ret != 0)
    {
        perror("pthread_create error");
        exit(1);
    }

    ret = pthread_join(thd, (void**)&retval);
    if(ret != 0)
    {
        perror("pthread_join error");
        exit(1);
    }

    printf("retval->var= %d, retval->str= %s\n", retval->var, retval->str);

    pthread_exit(NULL);
}
