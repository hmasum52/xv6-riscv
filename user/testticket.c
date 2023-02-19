#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    //printf("testticket.c\n");
    /* int n = atoi(argv[1]);
    if (settickets(n) < 0)
    {
        printf("settickets: error\n");
        exit(1);
    } */
    fork();
    while (1);
    exit(0);
}