#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<arpa/inet.h>


#define MAX_EVENTS 1024 //监听上限
#define SERV_PORT 9998 //端口号


//读数据
void recvdata(int fd, int events, void *arg);
//写数据
void senddata(int fd, int events, void *arg);

//描述就绪文件描述符相关信息
struct myevent_s
{
    int fd;             //要监听的文件描述符
    int events;         //对应监听事件
    void *arg;          //指向自己结构体的指针
    void (*call_back)(int fd, int events, void *arg);   //回调函数
    int status;         //是否在监听：1->在红黑树上(在监听)，0->不在红黑树上(不监听)
    char buf[BUFSIZ];
    int len;
    long last_active;   //记录每次加入红黑树g_efd的时间
};

int g_efd;      //全局变量，红黑树的根
struct myevent_s g_events[MAX_EVENTS + 1];      //自定义结构体类型数组。+1-->listenfd

//初始化结构体
void eventset(struct myevent_s *ev, int fd, void(*call_back)(int fd, int events, void *arg), void *arg)
{
    ev->fd = fd;
    ev->call_back = call_back;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;
    if(ev->len <= 0)
    {
        memset(ev->buf, 0, sizeof(ev->buf));
        ev->len = 0;
    }
    ev->last_active = time(NULL); //调用eventset函数的时间
    return;
}

//向epoll中添加红黑树 添加一个文件描述符
void eventadd(int efd, int events, struct myevent_s *ev)
{
    struct epoll_event epv={0, {0}};
    int op = 0;
    epv.data.ptr = ev;
    epv.events = ev->events = events;
    if(ev->status == 0)
    {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }
    if(epoll_ctl(efd, op, ev->fd, &epv) < 0)
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    return;
}

int setnonblock(int fd)
{
    int op = fcntl(fd, F_GETFL);
    op |= O_NONBLOCK;
    int flag;
    if((flag = fcntl(fd, F_SETFL, op)) < 0)
        printf("fcntl nonblocking failed, %s\n", strerror(errno));
    return flag;
}

//从epoll 监听的 红黑树中删除一个文件描述符
void eventdel(int efd, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};
    if(ev->status == 0)
        return;
    epv.data.ptr = NULL;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);
    return;
}

//当有文件描述符就绪，epoll返回，调用该函数与客户端建立连接
void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in cin;
    socklen_t socklen = sizeof(cin);
    int cfd, i;
    if((cfd = accept(lfd, (struct sockaddr*)&cin, &socklen)) == -1)
    {
        if(errno != EAGAIN && errno != EINTR)
            sleep(1);
        printf("%s:accept,%s\n", __func__, strerror(errno));
        return;
    }
    for(i = 0; i < MAX_EVENTS; ++i)
        if(g_events[i].status == 0)
            break;
    if(i == MAX_EVENTS)
    {
        printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
        return;
    }
    /* setnonblock(cfd); */
    eventset(&g_events[i], cfd, recvdata, &g_events[i]);
    eventadd(g_efd, EPOLLIN | 0, &g_events[i]);

    printf("new connect[%s:%d], [time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);
    return;  
}

//接收数据
void recvdata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s*)arg;

    int len = recv(fd, ev->buf, sizeof(ev->buf), 0);
    eventdel(g_efd, ev);
    if(len > 0)
    {
        ev->len = len;
        ev->buf[len] = '\0';
        printf("C[%d]:%s\n", fd, ev->buf);

        eventset(ev, fd, senddata, ev);
        eventadd(g_efd, EPOLLOUT | 0, ev);
    }
    else if(len == 0)
    {
        close(ev->fd);
        printf("[fd=%d] pos[%ld], closed\n", fd, ev-g_events);
    }
    else
    {
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }
    return;
}

//发送数据
void senddata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s*)arg;

    int len = send(fd, ev->buf, ev->len, 0);

    eventdel(g_efd, ev);

    if(len > 0)
    {
        printf("send[%d]:%s\n", fd, ev->buf);
        eventset(ev, fd, recvdata, ev);
        eventadd(g_efd, EPOLLIN | 0, ev);
    }
    else
    {
        close(ev->fd);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }
    return;
}

void initlistensocket(int efd, short port)
{
    struct sockaddr_in sin;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    /* setnonblock(lfd); */

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;

    if(bind(lfd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
    {
        perror("bind error");
        exit(1);
    }
    if(listen(lfd, 128) == -1)
    {
        perror("listen error");
        exit(1);
    }

    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);
    return;
}


int main(int argc, char*argv[])
{
    g_efd = epoll_create(MAX_EVENTS + 1);
    if(g_efd <= 0)
    {
        printf("create efd in %s err %s\n", __func__, strerror(errno));
        exit(1);
    }
    initlistensocket(g_efd, SERV_PORT);

    struct epoll_event events[MAX_EVENTS +1];
    printf("server running:port[%d]...\n", SERV_PORT);

    int i;
    while(1)
    {

        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        if(nfd < 0)
        {
            printf("epoll_wait error, exit\n");
            exit(1);
        }
        for(i = 0; i < nfd; ++i)
        {
            struct myevent_s *ev = (struct myevent_s*)events[i].data.ptr;
            if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
                ev->call_back(ev->fd, events[i].events, ev->arg);
            if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
                ev->call_back(ev->fd, events[i].events, ev->arg);
        }
    }


    return 0;
}
