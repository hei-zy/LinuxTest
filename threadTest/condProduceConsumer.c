#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>

void err_thread(int ret, char*str)
{
    if(ret != 0)
    {
        fprintf(stderr, "%s:%s\n", str, strerror(ret));
        pthread_exit(NULL);
    }
}

struct msg
{
    int num;
    struct msg*next;
};

struct msg*head;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;


void *producer(void*arg)
{
    while(1)
    {
        struct msg*mp = malloc(sizeof(struct msg));

        mp->num = rand() % 1000 + 1;
        printf("---produce %d\n", mp->num);

        pthread_mutex_lock(&mutex);
        mp->next = head;
        head = mp;
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&has_data);

        sleep(rand() % 3);
    }
    return NULL;
}

void *consumer(void*arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);
        while(head == NULL)
        {
            pthread_cond_wait(&has_data, &mutex);
        }

        struct msg*mp;
        mp = head;
        head = mp->next;
        pthread_mutex_unlock(&mutex);

        printf("---------consumer %d\n", mp->num);
        free(mp);
        sleep(rand()%3);
    }
    return NULL;
}


int main(int argc, char*argv[])
{
    int ret;
    pthread_t pid, cid;

    ret = pthread_create(&pid, NULL, producer, NULL);
    if(ret != 0)
        err_thread(ret, "pthread_create producer error");

    ret = pthread_create(&cid, NULL, consumer, NULL);
    if(ret != 0)
        err_thread(ret, "pthread_create consumer error");

    pthread_join(pid, NULL);
    pthread_join(cid, NULL);

    return 0;
}
