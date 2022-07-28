#include<sys/socket.h>

int get_line(int cfd, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while((i < size - 1) && (c != '\n'))
    {
        n = recv(cfd, &c, 1, 0);
        if(n > 0)
        {
            if(c == '\r')
            {
                n = recv(cfd, &c, 1, MSG_PEEK);
                if(n > 0 && c == '\n')
                    recv(cfd, &c, 1, 0);
                else
                    c = '\r';
            }
            buf[i] = c;
            ++i;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';

    if(n == -1)
        i = n;
    
    return i;
}
