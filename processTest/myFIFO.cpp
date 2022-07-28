#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<sys/stat.h>



int main(int argc, char*argv[])
{
    if(mkfifo("./myFIFO", 0664) == -1)
    {
        perror("mkfifo error\n");
        exit(1);
    }




    return 0;
}
