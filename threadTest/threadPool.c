#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/wait.h>



#define DEFAULT_TIME 10
#define MIN_WAIT_TASK_NUM 20
#define DEFAULT_THREAD_VARY 10

typedef struct
{
    void *(*function)(void*);       //函数指针，回调函数
    void *arg;                      //回调函数参数
}threadpool_task_t;                 //子线程任务结构体

typedef struct threadpool_t
{
    pthread_mutex_t lock;               //用于锁住本结构体
    pthread_mutex_t thread_counter;     //记录忙状态线程个数锁--busy_thr_num

    pthread_cond_t queue_not_full;      //当任务队列已满时，添加任务线程阻塞，等待此条件变量
    pthread_cond_t queue_not_empty;     //当任务队列不为空时，通知等待线程

    pthread_t *threads;                 //线程池中，存放每个线程的tid，数组
    pthread_t adjust_thread;            //管理线程tid
    threadpool_task_t *task_queue;     //任务队列

    int min_thr_num;
    int max_thr_num;
    int live_thr_num;
    int busy_thr_num;
    int wait_exit_thr_num;

    int queue_front;                    //任务队列队头下标
    int queue_rear;                     //任务队列队尾下标
    int queue_size;                     //任务队列实际任务数
    int queue_max_size;                 //任务队列最大任务数

    int shutdown;                       //标志位，线程使用状态，true or false
}threadpool_t;


void *threadpool_thread(void *threadpool);     //工作线程回调函数
void *adjust_thread(void *threadpool);         //管理线程回调函数

threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);//初始化线程池
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg);//向线程池中添加任务
int threadpool_free(threadpool_t *pool);//线程池空间释放
int threadpool_destroy(threadpool_t *pool);//销毁线程池


//工作线程回调函数
void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t*)threadpool;
    threadpool_task_t task;

    while(1)
    {
        pthread_mutex_lock(&(pool->lock));

        //queue_size == 0 没有任务，调wait阻塞任务队列有任务后唤醒
        while((pool->queue_size == 0) && (!pool->shutdown))
        {
            printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

            if(pool->wait_exit_thr_num > 0)
            {
                --pool->wait_exit_thr_num;

                //如果线程池中 线程数 大于 最小值 可以结束当前线程
                if(pool->live_thr_num > pool->min_thr_num)
                {
                    printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
                    --pool->live_thr_num;
                    pthread_mutex_unlock(&(pool->lock));

                    pthread_exit(NULL);
                }
            }

        }


        if(pool->shutdown)
        {
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        //从任务队列中获取任务，出队
        task.function = pool->task_queue[pool->queue_front].function;
        task.arg = pool->task_queue[pool->queue_front].arg;

        pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
        --pool->queue_size;

        //通知有任务已出队
        pthread_cond_signal(&(pool->queue_not_full));

        pthread_mutex_unlock(&(pool->lock));

        //开始执行任务
        printf("thread 0x%d start working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        ++pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        task.function(task.arg);        //执行回调函数处理任务

        //任务处理结束
        printf("thread 0x%d end working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        --pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));
    }

    pthread_exit(NULL);
}

void *adjust_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t*)threadpool;
    while(!pool->shutdown)
    {
        sleep(DEFAULT_TIME);

        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;
        int live_thr_num = pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num = pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        //创建新线程：   任务数 大于 最小线程数，且存活线程数 少于 最大线程数
        if(queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num)
        {
            pthread_mutex_lock(&(pool->lock));
            int add = 0;

            for(int i = 0; i < pool->max_thr_num
                    && add < DEFAULT_THREAD_VARY
                    && pool->live_thr_num < pool->max_thr_num;
                    ++i)
            {
                if(pool->threads[i] == 0 /*|| !is_thread_alive(pool->threads[i])*/)
                {
                    pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool);
                    ++add;
                    ++pool->live_thr_num;
                }

            }
            pthread_mutex_unlock(&(pool->lock));
        }
        
        //销毁多余空闲线程：   忙线程x2 小于 存活线程数，且存活线程数 少于 最大线程数
        if((busy_thr_num * 2) < live_thr_num && live_thr_num > pool->min_thr_num)
        {
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));

            for(int i = 0; i < DEFAULT_THREAD_VARY; ++i)
            {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }


    }
 

    return NULL;
}


//初始化线程池
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
    threadpool_t *pool = NULL;

    do
    {
        if((pool = (threadpool_t*)malloc(sizeof(threadpool_t))) == NULL)
        {
            printf("malloc threadpool fail\n");
            break;
        }
        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->live_thr_num = min_thr_num;
        pool->busy_thr_num = 0;
        pool->wait_exit_thr_num = 0;
        pool->queue_front = 0;
        pool->queue_rear = 0;
        pool->queue_size = 0;
        pool->queue_max_size = queue_max_size;
        pool->shutdown = 0;

        //根据最大线程上限，给工作数组开辟空间，并清零
        pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * max_thr_num);
        if(pool->threads == NULL)
        {
            printf("malloc threads fail\n");
            break;
        }
        memset(pool->threads, 0, sizeof(pthread_t) * max_thr_num);

        //给任务队列开辟空间
        pool->task_queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t*) * queue_max_size);
        if(pool->task_queue == NULL)
        {
            printf("malloc task_queue fail\n");
            break;
        }

        //初始化互斥锁、条件变量
        if(pthread_mutex_init(&(pool->lock), NULL) != 0
            || pthread_mutex_init(&(pool->thread_counter), NULL) != 0
            || pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
            || pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
        {
            printf("init the lock or cond fail\n");
            break;
        }
    
        //启动 min_thr_num 个工作线程
        for(int i = 0; i < min_thr_num; ++i)
        {
            pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool);  //pool指向当前线程池
        }
        //创建管理者线程
        pthread_create(&(pool->adjust_thread), NULL, adjust_thread, (void*)pool);

        return pool;
    }while(0);

    //线程池创建失败，释放pool空间
    threadpool_free(pool);

    return NULL;
}


//向线程池中添加任务
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg)
{
    pthread_mutex_lock(&(pool->lock));
    
    //队列已满，调wait阻塞
    while((pool->queue_size == pool->queue_max_size) && (!pool->shutdown))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
    }
    
    if(pool->shutdown)
    {
        pthread_cond_broadcast(&(pool->queue_not_empty));
        pthread_mutex_unlock(&(pool->lock));
        return 0;
    }

    //清空回调函数的参数arg
    if(pool->task_queue[pool->queue_rear].arg != NULL)
        pool->task_queue[pool->queue_rear].arg = NULL;

    //添加任务到任务队列
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;       //环形队列
    ++pool->queue_size;

    //添加任务成功后，唤醒线程池中 等待处理任务的线程
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

//销毁线程池
int threadpool_destroy(threadpool_t *pool)
{
    if(pool == NULL)
        return -1;

    pool->shutdown = 1;

    //销毁管理线程
    pthread_join(pool->adjust_thread, NULL);

    //销毁其他线程
    for(int i = 0; i < pool->live_thr_num; ++i)
        pthread_cond_broadcast(&(pool->queue_not_empty));
    for(int i = 0; i < pool->live_thr_num; ++i)
        pthread_join(pool->threads[i], NULL);

    threadpool_free(pool);
    return 0;
}

//释放线程池空间
int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL)
        return -1;

    if(pool->task_queue)
        free(pool->task_queue);
    if(pool->threads)
    {
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool = NULL;
    return 0;
}


void*process(void *arg)
{
    int i = *(int*)(arg);
    printf("do work: %d\n", i);
    return NULL;
}

int main(int argc, char*argv[])
{
    threadpool_t *thp = threadpool_create(3, 100, 100);
    printf("pool inited");

    int num[20], i;
    for(i = 0; i < 20; ++i)
    {
        num[i] = i;
        printf("add task %d\n", i);
        threadpool_add(thp, process, (void*)&num[i]);
    }
    
    sleep(10);
    threadpool_destroy(thp);

    return 0;
}
