#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<cerrno>

#define MSG_TRY "tryAgain\n"

int main(int argc,char*argv[])
{
    char buf[1024];
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    if(flag == -1)
    {
        perror("fcntl error");
        exit(1);
    }
    flag |= O_NONBLOCK;
    int ret = fcntl(STDIN_FILENO, F_SETFL, flag);
    if(ret == -1)
    {
        perror("fcntl error");
        exit(1);
    }

    int length;
    while(length = read(STDIN_FILENO, buf, strlen(buf)), length < 0)
    {
        if(errno != EAGAIN)
        {
            perror("read /dev/tyy");
            exit(1);
        }
        sleep(3);
        write(STDOUT_FILENO, MSG_TRY, strlen(MSG_TRY));
    }
    write(STDOUT_FILENO, buf, length);
    


    return 0;
}

