/**
 * Author: Hasan Masum. github/hmasum52
*/

#include<kernel/types.h>
#include<kernel/stat.h>
#include<user/user.h>

int main(){
    printf("Enter a number: ");
    char buf[10];
    gets(buf, 9); // avoid new line
    int num = atoi(buf);
    printf("%d^2 = %d\n", num, num*num);
    return 0;
}