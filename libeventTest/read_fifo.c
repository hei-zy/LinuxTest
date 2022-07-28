#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<event2/event.h>


#define FIFOPATH "myfifo"

void read_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[1024];

    int len = read(fd, buf, sizeof(buf));
    if(len == -1)
    {
        perror("read error");
        exit(1);
    }

    printf("read event: %s \n", what & EV_READ ? "Yes" : "NO");
    printf("data len = %d, buf = %s\n", len, buf);
    sleep(1);
}


int main(int argc, char*argv[])
{
    unlink(FIFOPATH);
    mkfifo(FIFOPATH, 0664);
    

    int fd = open(FIFOPATH, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        perror("open error");
        exit(1);
    }

    struct event_base *base = event_base_new();

    struct event *ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

    event_add(ev, NULL);

    event_base_dispatch(base);

    event_free(ev);
    event_base_free(base);

    close(fd);

    return 0;
}
