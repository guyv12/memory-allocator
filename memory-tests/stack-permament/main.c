#include <stdio.h>
#include <stdlib.h>

extern void switch_stack();

void recursion()
{
    static int num = 0;
    
    printf("I am %d function called on a new stack\n", num++);
    recursion();
}


int main(void)
{
    switch_stack();

    recursion();

    return 0;
}