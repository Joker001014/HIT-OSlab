#define __LIBRARY__
#include <unistd.h>

_syscall2(int, whoami, char*, name, unsigned int, size);

int main(int argc, char* agrv[])
{
    char s[30];
    whoami(s, 30);
    printf("%s\n", s);
    
    return 0;
}



