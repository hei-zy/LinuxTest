#include<stdio.h>
#include<stdlib.h>
#include<cerrno>
#include<cstring>
#include<dirent.h>
#include<unistd.h>
#include<sys/stat.h>

void isFile(char*dir);
void readDir(char*dir);

int main(int argc,char*argv[])
{
    if(argc == 1)
        isFile(".");
    else
        isFile(argv[1]);

    return 0;
}



void readDir(char*dir)
{
    char path[256];
    DIR* dp = opendir(dir);
    if(dp == NULL)
    {
        perror("opendir error");
        return;
    }
    struct dirent*sdp;
    while(sdp = readdir(dp), sdp != NULL)
    {
        if(strcmp(sdp->d_name, ".") == 0 || strcmp(sdp->d_name, ".") == 0)
        {
            continue;    
        }
        sprintf(path, "%s/%s", dir, sdp->d_name);
        isFile(path);
    }
    closedir(dp);
}

void isFile(char*dir)
{
    struct stat ss;
    int ret = stat(dir, &ss);
    if(ret == -1)
    {
        perror("stat error");
        return;
    }
    if(S_ISDIR(ss.st_mode))
    {
        readDir(dir);
        return;
    }

    printf("%10s\t\t%ld\n", dir, ss.st_size);
}
