#include<kernel/types.h>
#include<kernel/stat.h>
#include<user/user.h>

int main(int argc, char const *argv[])
{
    printf("System call getname() test...\n");
    getname();
    return 0;
}
