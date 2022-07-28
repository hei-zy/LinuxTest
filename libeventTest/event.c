#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>
#include<event2/event.h>


void sys_err(const char *str)
{
    perror(str);
    exit(1);
}



int main(int argc, char *argv[])
{
    struct event_base *base = event_base_new();

    const char *buf;

    buf = event_base_get_method(base);
    printf("%s\n", buf);


    return 0;
}
