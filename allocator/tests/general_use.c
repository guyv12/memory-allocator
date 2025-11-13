#include "arena.h"
#include <stdio.h>

typedef struct SampleStruct SampleStruct;

struct SampleStruct
{
        int d;
        float f;
        char c[10];
};

int main(int argc, char *argv[])
{
    MyArena Arena;

    InitArena(&Arena, 20);

    // double *arr = (double *)ArenaAllocate(&Arena, 2 * sizeof(double));
    // arr[0] = 2.0; arr[1] = 0.5;
    // printf("%lf, %lf\n", arr[0], arr[1]);

    SampleStruct *my_struct = (SampleStruct *)ArenaAllocate(&Arena, sizeof(SampleStruct));
    my_struct->d = 10; my_struct->f = 0.5; my_struct->c[0] = 'a';
    printf("%d, %f, %s\n", my_struct->d, my_struct->f, my_struct->c);

    FreeArena(&Arena);
    
    return 0;
}