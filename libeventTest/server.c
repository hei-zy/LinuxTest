#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>

#define PORT 9527


void readcb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};

    bufferevent_read(bev, buf, sizeof(buf));
    printf("client say: %s\n", buf);

    char *p = "我是服务器，已成功收到数据！\n";

    bufferevent_write(bev, p, strlen(p) + 1);
    sleep(1);
}

void writecb(struct bufferevent* bev, void *arg)
{
    printf("已成功给客户端返回数据...\n");
}

void eventcb(struct bufferevent* bev, short events, void*arg)
{
    if(events & BEV_EVENT_EOF)
        printf("connection closeed\n");
    else if(events & BEV_EVENT_ERROR)
        printf("some other error\n");

    bufferevent_free(bev);

    printf("bufferevent 资源已经被释放...\n");
}


void cb_listener(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr* addr,
        int len, void *ptr)
{
    printf("new client connected\n");

    struct event_base*base = (struct event_base*)ptr;

    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, readcb, writecb, eventcb, NULL);

    bufferevent_enable(bev, EV_READ);
}

int main(int argc, char*argv[])
{
    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    struct event_base *base = event_base_new();

    struct evconnlistener *listener = evconnlistener_new_bind(base, cb_listener, base,
            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
            -1, (struct sockaddr*)&serv, sizeof(serv));
    
    event_base_dispatch(base);

    evconnlistener_free(listener);

    event_base_free(base);


    return 0;
}
