// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N 1000

void print(const char *s)
{
    write(1, s, strlen(s));
}

void forktest(void)
{
    int *ptr = malloc(sizeof(int));

    stats();

    *ptr = 0;
    printf("initial value: %d\n", *ptr);

    int pid = fork();
    // child
    if (pid == 0)
    {
        stats();
        *ptr = 1;
        printf("child: %d\n", *ptr);
        stats();
    }
    else
    {
        wait(0);
        stats();
        *ptr = 2;
        printf("parent: %d\n", *ptr);
        stats();
    }
}

int main(void)
{
    forktest();
    exit(0);
}
