#include<stdio.h>
#include<stdlib.h>
#include<error.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<string.h>


struct student
{
    int id;
    char name[256];
    int age;
};


int var = 1234;
int main(int argc, char*argv[])
{
    int fd = open("temp", O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd < 0)
    {
        perror("mmap error");
        exit(1);
    }


    struct student stu = {1, "xiaoming", 18};
    ftruncate(fd, sizeof(stu));
    struct student *p = (struct student*)mmap(NULL, sizeof(stu), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(p == MAP_FAILED)
    {
        perror("mmap error");
        exit(1);
    }
    close(fd);

    
    while(1)
    {
        memcpy(p, &stu, sizeof(stu));
        ++stu.id;
        sleep(1);
    }

    munmap(p, sizeof(stu));

    return 0;
}
