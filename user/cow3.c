#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"

int main(){
    int* c = malloc(sizeof(int));

    int pid = fork();
    stats();
    if(pid == 0){
        *c = 1;
        printf("c = %d\n", *c);
        stats();
        exit(0);
    }
    else{
        wait(0);
        *c = 2;
        printf("c= %d\n", *c);
        stats();
        exit(0);
    }
}