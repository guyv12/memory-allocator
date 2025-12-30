#include "arena.h"
#include <stdio.h>
#include <stddef.h>
#include <stdalign.h>

typedef struct SampleStruct
{
        int d;
        float f;
        char c[10];
}
SampleStruct;

int main(int argc, char *argv[])
{
    ArenaAllocator Arena;

    arcreate(&Arena, 20, ARENA_GROW);

    SampleStruct *my_struct = (SampleStruct *)aralloc(&Arena, sizeof(SampleStruct));
    my_struct->d = 10; my_struct->f = 0.5; my_struct->c[0] = 'a';
    printf("%d, %f, %s\n", my_struct->d, my_struct->f, my_struct->c);

    size_t sub_mark = armark(&Arena);
    my_struct = aligned_aralloc(&Arena, 16, sizeof(SampleStruct));
    my_struct->d = 20; my_struct->f = 2.0; my_struct->c[0] = 'b';
    printf("%d, %f, %s\n", my_struct->d, my_struct->f, my_struct->c);
    arrollback(&Arena, sub_mark);

    double *arr = aralloc(&Arena, 2 * sizeof(double));
    arr[0] = 0.5; arr[1] = 1.5;
    printf("%lf, %lf\n", arr[0], arr[1]);

    ardestroy(&Arena);
    
    return 0;
}