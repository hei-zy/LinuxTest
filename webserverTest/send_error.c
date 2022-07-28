#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>

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


#define ERR_HTML "404.html"
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

