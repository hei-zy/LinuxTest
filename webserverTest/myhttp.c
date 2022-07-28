#include<stdio.h>
#include<dirent.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<ctype.h>

#define MAX_SIZE 2048
#define FILE_SIZE 10240
#define ERR_HTML "404.html"


//十六进制 转 十进制
int hexit(char c)
{
    if(c >= '0' && c <= '9')
        return c - '0';
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

//编码
void encode_str(char *to, int tosize, const char*from)
{
    int tolen;

    for(tolen = 0; *from != '\0' && tolen +4 < tosize; ++from)
    {
        if(isalnum(*from) || strchr("/_.-~", *from) != (char*)0)
        {
            *to = *from;
            ++to;
            ++tolen;
        }
        else
        {
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }
    }
    *to= '\0';
}

//解码
void decode_str(char *to, char *from)
{
    for(; *from != '\0'; ++to, ++from)
    {
        if(from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            *to = hexit(from[1])*16 + hexit(from[2]);
            from += 2;
        }
        else
        {
            *to = *from;
        }
    }
    *to = '\0';
}

//发送错误页面
void send_errorB(int cfd, int status, char *title, char *text)
{
    char buf[4096] = {0};

    sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
    sprintf(buf + strlen(buf), "Content-Type:%s\r\n", "text/html");
    sprintf(buf+strlen(buf), "Content-Length:%d\r\n", -1);
    sprintf(buf+strlen(buf), "Connection: close\r\n");
    send(cfd, buf, strlen(buf), 0);
    send(cfd, "\r\n", 2, 0);

    memset(buf, 0, sizeof(buf));

    int err_fd = open(ERR_HTML, O_RDONLY);

    int n;
    while((n = read(err_fd, buf, sizeof(buf))) > 0)
    {
        send(cfd, buf, n, 0);
    }
    close(err_fd);

    return;
}



//发送错误页面
void send_error(int cfd, int status, char *title, char *text)
{
    char buf[10240] = {0};

    sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
    sprintf(buf + strlen(buf), "Content-Type:%s\r\n", "text/html");
    sprintf(buf+strlen(buf), "Content-Length:%d\r\n", -1);
    sprintf(buf+strlen(buf), "Connection: close\r\n");
    send(cfd, buf, strlen(buf), 0);
    send(cfd, "\r\n", 2, 0);

    memset(buf, 0, sizeof(buf));

    sprintf(buf, "<html><head><title>%d %s</title></head>\n", status, title);
    sprintf(buf+strlen(buf), "<body bgcolor=\"0,235,255\"><h2 align=\"center\">");
    sprintf(buf+strlen(buf), "<font style=\"font-size:55px\"><b>%d %s</b></font></h2>\n", status, title);
    sprintf(buf+strlen(buf),"<div align=center>");
    sprintf(buf+strlen(buf), "<font style=\"font-size:20px\"><b>%s\n</b></font>", text);
    sprintf(buf+strlen(buf), "</div>");
    sprintf(buf+strlen(buf), "<hr>\n<br>\n");

    sprintf(buf+strlen(buf),"<div align=center>");
    sprintf(buf+strlen(buf), "<a href=\"https://baidu.com\" title=\"go to baidu\" target=\"_blank\">");
    sprintf(buf+strlen(buf), "<img src=\"./rc/girl.jpg\" alt=\"图片加载失败\" title=\"go to baidu\" width=\"500\">");
    sprintf(buf+strlen(buf), "</a>");
    sprintf(buf+strlen(buf), "</div>");

    sprintf(buf+strlen(buf),"<div align=\"right\">");
    sprintf(buf+strlen(buf), "<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n");
    sprintf(buf+strlen(buf), "<a href=\"#top\" title=\"top\">");
    sprintf(buf+strlen(buf), "<img src=\"./rc/top.png\" alt=\"加载失败\" title=\"top\" width=\"30\">");
    sprintf(buf+strlen(buf), "</a>");
    sprintf(buf+strlen(buf), "</div>");

    sprintf(buf+strlen(buf), "</body>\n</html>\n");
    send(cfd, buf, strlen(buf), 0);

    return;
}

char *get_file_type(const char *name)
{
    char *dot;

    //自右向左查找‘.’字符，如不存在返回NULL
    dot = strrchr(name, '.');
    if(dot == NULL)
        return "text/plain; charset=utf-8";
    if(strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if(strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if(strcmp(dot, ".gif") == 0)
        return "image/gif";
    if(strcmp(dot, ".png") == 0)
        return "image/png";

    if(strcmp(dot, ".css") == 0)
        return "text/css";
    if(strcmp(dot, ".au") == 0)
        return "audio/basic";
    if(strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if(strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";


    if(strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if(strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if(strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if(strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if(strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if(strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if(strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

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

int init_listen_fd(int port, int epfd)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socker error");
        exit(1);
    }

    struct sockaddr_in sockaddr;
    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //设置端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int ret = bind(lfd, (struct sockaddr*)&sockaddr,sizeof(sockaddr));
    if(ret == -1)
    {
        perror("bind error");
        exit(1);
    }

    ret = listen(lfd, -1);
    if(ret == -1)
    {
        perror("listen error");
        exit(1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl add lfd error");
        exit(1);
    }

    return lfd;
}

void disconnect(int cfd, int epfd)
{
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    if(ret != 0)
    {
        //回发404页面
        perror("epoll_ctl error");
        exit(1);
    }
    close(cfd);
}

//客户端的 fd，错误号，错误描述，回发文件类型，文件长度
void send_respond(int cfd, int num, char *disp, char *type, int len)
{
    char buf[1024] = {0};
    sprintf(buf, "HTTP/1.1 %d %s\r\n", num, disp);
    sprintf(buf + strlen(buf), "%s\r\n", type);
    sprintf(buf + strlen(buf), "Content-length:%d\r\n", len);
    send(cfd, buf, strlen(buf), 0);
    send(cfd, "\r\n", 2, 0);
}

//发送本地服务器的文件 给浏览器
void send_file(int cfd, const char* file)
{
    int fd = open(file, O_RDONLY);
    if(fd == -1)
    {
        perror("open error");
        return;
    }

    char buf[FILE_SIZE] = {0};
    int n, ret;
    while((n = read(fd, buf, sizeof(buf))) > 0)
    {
        if(n == -1)
        {
            if(errno == EAGAIN)
            {
                continue;
            }
            else if(errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("read error");
                break;
            }
        }
        ret = send(cfd, buf, n, 0);
        if(ret == -1)
        {
            if(errno == EAGAIN)
            {
                continue;
            }
            else if(errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("send error");
                break;
            }
        }
    }

    close(fd);
}

void send_dir(int cfd, const char* dirname)
{
    int i,ret;

    char buf[4096] = {0};

    char enstr[1024] = {0};
    char path[1024] = {0};

    sprintf(buf, "<html><head><title>DIR: %s</title></head>", dirname);
    sprintf(buf+strlen(buf), "<body><h1>CurDIR: %s</h1></table>", dirname);
    
    //目录项二级指针
    struct dirent**ptr;
    int num = scandir(dirname, &ptr, NULL, alphasort);

    //遍历
    for(i = 0; i < num; ++i)
    {
        char*name = ptr[i]->d_name;

        //拼接文件路径
        sprintf(path, "%s/%s", dirname, name);
        printf("path = %s ================\n", path);
        struct stat st;
        stat(path, &st);

        encode_str(enstr, sizeof(enstr), name);

        if(S_ISREG(st.st_mode)) //文件
        {
            sprintf(buf+strlen(buf),
                    "<tr><td><a href=\"%s\">%s</a></td><td>%ld<br/></td></tr>",
                    enstr, name, (long)st.st_size);
        }else if(S_ISDIR(st.st_mode))  //目录
        {
            sprintf(buf+strlen(buf),
                    "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld<br/></td></tr>",
                    enstr, name, (long)st.st_size);
        }
        ret = send(cfd, buf, strlen(buf), 0);
        if(ret == -1)
        {
            if(errno == EAGAIN)
            {
                perror("send error");
                continue;
            }
            else if(errno == EINTR)
            {
                perror("send error");
                continue;
            }
            else
            {
                perror("send error");
                exit(1);
            }
        }
        memset(buf, 0, sizeof(buf));
    }
    sprintf(buf+strlen(buf),"<div align=\"right\">");
    sprintf(buf+strlen(buf), "<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n<br>\n");
    sprintf(buf+strlen(buf), "<a href=\"#top\" title=\"top\">");
    sprintf(buf+strlen(buf), "<img src=\"./rc/top.png\" alt=\"Load Fail\" title=\"top\" width=\"30\">");
    sprintf(buf+strlen(buf), "</a>");
    sprintf(buf+strlen(buf), "</div>");

    sprintf(buf+strlen(buf), "</table></body></html>");
    send(cfd, buf, strlen(buf), 0);;

    printf("dir message send OK!!!\n");
}

void http_request(int cfd, const char *file)
{
    /* //转码 解码 */
    /* decode_str(path, path); */
    /*  */
    /* char*file = path + 1; */
    /*  */
    /* if(strcmp(path, '/') == 0) */
    /*     file = "./"; */
    /*  */
    struct stat sbuf;

    //判断文件是否存在
    int ret = stat(file, &sbuf);
    if(ret == -1)
    {
        printf("no this file\n");
        //回发404页面
        send_error(cfd, 404, "Not Found", "NO such file or direntry");
        return;
    }

    if(S_ISREG(sbuf.st_mode)) //文件
    {
        //回发http应答
        send_respond(cfd ,200, "OK", get_file_type(file), sbuf.st_size);
        //回发给客户端请求数据
        send_file(cfd, file);
    }
    else if(S_ISDIR(sbuf.st_mode))  //目录
    {
        send_respond(cfd ,200, "OK", get_file_type(".html"), -1);
        send_dir(cfd, file);
    }
}

void do_read(int cfd, int epfd)
{
    char line[1024] = {0};
    int len = get_line(cfd, line, sizeof(line));
    if(len == 0)
    {
        printf("服务器，检测到客户端关闭...\n");
        disconnect(cfd, epfd);
    }
    else if(len == -1)
    {
        perror("get_line error");
    }
    else
    {
        char method[16], path[256], protocol[16];
        sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
        printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);

        while(1)
        {
            char buf[1024] = {0};
            len = get_line(cfd, buf, sizeof(buf));
            if(buf[0] == '\0')
                break;
            else if(len == -1)
                break;
        }
        if(strncasecmp(method, "GET", 3) == 0)
        {
            decode_str(path, path);
            char *file = path + 1; //提取文件名
            
            if(strcmp(path, "/") == 0)
                file = "./";
            http_request(cfd, file);

            //关闭套接字
            disconnect(cfd, epfd);
        }
    }
}

void do_accept(int lfd, int epfd)
{
    struct sockaddr_in clt_addr;
    socklen_t clt_addr_len = sizeof(clt_addr);

    int cfd = accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
    if(cfd == -1)
    {
        perror("accept error");
        exit(1);
    }

    char client_ip[64] = {0};
    printf("NEW Client IP: %s, Port: %d, cfd = %d\n",
            inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
            ntohs(clt_addr.sin_port), cfd);

    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    struct epoll_event ev;
    ev.data.fd = cfd;

    ev.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl add cfd error");
        exit(1);
    }
}

void epoll_run(int port)
{
    int epfd = epoll_create(MAX_SIZE);
    if(epfd == -1)
    {
        perror("epoll_create error");
        exit(1);
    }

    int lfd = init_listen_fd(port, epfd);

    struct epoll_event all_events[MAX_SIZE];
    int i = 0;
    while(1)
    {
        int ret = epoll_wait(epfd, all_events, MAX_SIZE, -1);
        if(ret == -1)
        {
            perror("epoll_wait error");
            exit(1);
        }

        for(i = 0; i < ret; ++i)
        {
            struct epoll_event *pev = &all_events[i];

            if(!(pev->events & EPOLLIN))
                continue;

            if(pev->data.fd == lfd)
                do_accept(lfd, epfd);
            else
                do_read(pev->data.fd, epfd);
        }
    }
}


int main(int argc,char*argv[])
{
    if(argc < 3)
    {
        printf("./server port path\n");
        exit(1);
    }

    //获取用户输入的端口
    int port = atoi(argv[1]);

    //改变进程工作目录
    int ret = chdir(argv[2]);
    if(ret != 0)
    {
        perror("chdir error");
        exit(1);
    }

    //启动epoll监听
    epoll_run(port);

    return 0;
}
