#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>

#define PORT 9527


void readcb(struct bufferevent*bev, void *arg)
{
    char buf[1024] = {0};
    bufferevent_read(bev, buf, sizeof(buf));

    printf("fwq say:%s\n", buf);

    sleep(1);
}

void eventcb(struct bufferevent* bev, short events, void*arg)
{
    if(events & BEV_EVENT_EOF)
        printf("connection closeed\n");
    else if(events & BEV_EVENT_ERROR)
        printf("some other error\n");
    else if(events & BEV_EVENT_CONNECTED)
    {
        printf("已成功连接服务器...\n");
        return;
    }

    bufferevent_free(bev);

    printf("bufferevent 资源已经被释放...\n");
}

void read_terminal(evutil_socket_t fd, short what, void *arg)
{
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf));

    struct bufferevent*bev = (struct bufferevent*)arg;
    bufferevent_write(bev, buf, len + 1);
}


int main(int argc, char*argv[])
{
    struct event_base *base = event_base_new();

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct bufferevent *bev = NULL;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);

    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));

    bufferevent_setcb(bev, readcb, NULL, eventcb, NULL);
    
    bufferevent_enable(bev, EV_READ);
    
    struct event*ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
            read_terminal, bev);

    event_add(ev, NULL);

    event_base_dispatch(base);

    event_free(ev);

    event_base_free(base);


    return 0;
}
