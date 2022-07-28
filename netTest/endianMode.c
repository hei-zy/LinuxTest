#include<stdio.h>

void byteOrder()
{
    union
    {
        short val;
        char unionBytes[sizeof(short)];
    }test;
    test.val = 0x0102;
    if(test.unionBytes[0] == 1 && test.unionBytes[1] == 2)
        printf("big endian\n");
    else if(test.unionBytes[0] == 2 && test.unionBytes[1] == 1)
        printf("little endian\n");
    else
        printf("unknow...\n");
}


int main(int argc, char*argv[])
{
    byteOrder();


    return 0;
}
