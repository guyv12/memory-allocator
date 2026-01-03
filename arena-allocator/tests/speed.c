#include "arena.h"
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <time.h>


typedef struct SampleStruct
{
        int d;
        float f;
        char c[10];
}
SampleStruct;

int main(int argc, char *argv[])
{
    int N = pow(10, 9);

    ArenaAllocator Arena;
    arcreate(&Arena, sizeof(double), 0);
    size_t mark = armark(&Arena);
    double total_sum = 0;

    clock_t start = clock();

    for (int i = 0; i < N; i++)
    {
        volatile double *d = aralloc(&Arena, sizeof(double));
        *d = i; 
        total_sum += *d;
        arrollback(&Arena, mark);
    }

    clock_t end = clock();
    printf("arena time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);

    total_sum = 0;
    start = clock();

    for (int i = 0; i < N; i++)
    {
        volatile double *d = malloc(sizeof(double));
        *d = i;
        total_sum += *d;
        free(d);
    }

    end = clock();
    printf("malloc time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);

    // -O3 flag
    // 1445ms for arena 2555ms for malloc

    return 0;
}