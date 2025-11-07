#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// adds 5 to a and does some syscall on malloced stack
extern void sys(int *a);

int main()
{
    int a = 10;
    sys(&a);

    printf("\na: %d\n", a);

    return 0;
}