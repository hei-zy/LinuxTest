#include<stdio.h>
#include<cstdlib>
#include<cstring>

int main(int argc, char*argv[])
{
    char buf[111] = "dasfa";

    printf("sizeof(buf) = %d\n", static_cast<int>(sizeof(buf)));

    printf("strlen(buf) = %d\n", (int)strlen(buf));

    return 0;
}
