#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include "arena.h"


typedef struct SomeData {
    char a;
    int b;
    double c;
}
SomeData;


int main(int argc, char *argv[])
{
    size_t size = sizeof(SomeData);
    size_t alignment = alignof(SomeData);
    
    printf("struct size: %ld\n", size); 
    printf("struct alignment: %ld\n", alignment);


    printf("Benchmark: \n");
    
    int N = pow(10, 9);
    size_t buffer_size = (N + 1) * sizeof(SomeData);
    ArenaAllocator arena1, arena2;
    arcreate(&arena1, buffer_size, 0); arcreate(&arena2, buffer_size, 0);

    SomeData *aligned = aligned_aralloc(&arena1, alignment, buffer_size);
    SomeData *unaligned = (SomeData *)((uint8_t *)aligned_aralloc(&arena2, alignment, buffer_size) + 1); // shift by 1 pointer (the data read is not important)

    volatile double sum1 = 0;
    volatile double sum2 = 0;


    //----- Aligned test ----

    clock_t start = clock();

    for (int i = 0; i < N; i++)
    {
        sum1 += aligned[i].c; // GRAB ONLY DOUBLE CUZ THAT WILL HAVE THE SHIFT GRABBING 8BYTES
    }

    clock_t end = clock();

    printf("aligned access time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);


    //----- Unaligned test -----

    start = clock();

    for (int i = 0; i < N; i++)
    {
        sum2 += unaligned[i].c;
    }

    end = clock();

    printf("unaligned access time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);


    // around 2143ms for aligned and 2158 for unaligned

    ardestroy(&arena1); ardestroy(&arena2);
    return 0;
}