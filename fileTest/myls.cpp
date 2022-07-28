#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<cerrno>
#include<stdlib.h>
#include<cstring>



int main(int argc,char*argv[])
{
    DIR * pd;
    pd = opendir(argv[1]);
    if(pd == NULL)
    {
        perror("opendir error\n");
        exit(1);
    }

    struct dirent* sdp;
    while(sdp = readdir(pd), sdp != NULL)
    {
        if(strcmp(sdp->d_name, ".") == 0 || strcmp(sdp->d_name, "..") == 0)
        {
            continue;
        }
        printf("%s\t", sdp->d_name);
    }
    printf("\n");

    closedir(pd);

    return 0;
}
