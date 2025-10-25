#include <stdio.h>
#include <stdlib.h>

extern void stack_test(int *p);

void fastcall_adder(int a, int b, int c, int d, int e, int f, int *p)
{
    (*p)++;
}

int main()
{
    int a = 10;
    stack_test(&a);

    printf("a: %d\n", a);

    return 0;
}