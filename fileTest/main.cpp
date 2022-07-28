#include<cstdio>
#include<unistd.h>
#include<fcntl.h>
#include<cstdlib>

int main(int argc,char*argv[])
{
    for(int i = 0; i < argc; ++i)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    char buf[1024];
    int fl1 = open(argv[1],O_RDONLY);
    int fl2 = open(argv[2],O_RDWR | O_CREAT | O_TRUNC,0664);

    if( fl1 == -1 || fl2 == -1)
    {
        perror("file open fatal\n");
        exit(-1);
    }

    int fsize;
    while((fsize = read(fl1, buf, sizeof(buf))) != 0)
    {
        if(fsize == -1)
        {
           perror("file read fatal\n");
           exit(-1);
        }
        write(fl2, buf, fsize);
    }
    
    close(fl1);
    close(fl2);

    return 0;
}
