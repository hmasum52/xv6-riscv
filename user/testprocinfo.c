#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

void print_formated(int n, int width){
    int size = n? 0:1;
    int orginal = n;
    while(n){
        int d = n%d;
        n /= 10;
        size++;
    }
    int space = width - size;
    space = space > 0 ? space:0;
    printf(" %d", orginal);
    while(space--) printf(" ");
}

int main(int argc, char *argv[])
{
   // printf("testprocinfo.c\n");
    struct pstat ps;
    if(getpinfo(&ps) < 0 ){
        printf("getpinfo: error\n");
        exit(1);
    }
    
    // print table
    printf(" PID | In Use | Original Tickets | Current Tickets | Time Slices\n");
    for(int i=0; i<NPROC; i++){
        // print row
        if(ps.inuse[i]){
            print_formated(ps.pid[i], 5);
            print_formated(ps.inuse[i], 8);
            print_formated(ps.tickets_original[i], 18);
            print_formated(ps.tickets_current[i], 17);
            print_formated(ps.time_slices[i], 0);
            printf("\n");
        }
        
    }
    exit(0);
}