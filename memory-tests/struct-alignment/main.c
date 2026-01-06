#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <time.h>
#include <stdint.h>
#include <math.h>


typedef struct SomeData {
    char a;
    int b;
    double c;
}
SomeData;

void aligned()
{
    size_t size = sizeof(SomeData);
    size_t alignment = alignof(SomeData);
    
    printf("struct size: %ld\n", size); 
    printf("struct alignment: %ld\n", alignment);


    printf("Benchmark: \n");
    int N = pow(10, 9);
    size_t buffer_size = (N + 1) * sizeof(SomeData);

    SomeData *aligned = aligned_alloc(alignment, buffer_size);

    volatile double sum = 0;


    //----- Aligned test ----

    clock_t start = clock();

    for (int j = 0; j < 10; j++)
    {
        for (int i = 0; i < N; i++)
        {
            sum += aligned[i].c; // GRAB ONLY DOUBLE CUZ THAT WILL HAVE THE SHIFT GRABBING 8BYTES
        }
    }

    clock_t end = clock();

    printf("aligned access time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);
    free(aligned);
}


void unaligned()
{
    size_t size = sizeof(SomeData);
    size_t alignment = alignof(SomeData);
    
    printf("struct size: %ld\n", size); 
    printf("struct alignment: %ld\n", alignment);


    printf("Benchmark: \n");
    int N = pow(10, 9);
    size_t buffer_size = (N + 1) * sizeof(SomeData);

    SomeData *unaligned = (SomeData *)((uint8_t *)aligned_alloc(alignment, buffer_size) + 1); // shift by 1 pointer (the data read is not important)

    volatile double sum = 0;


    //----- Aligned test ----

    clock_t start = clock();

    for (int j = 0; j < 10; j++)
    {
        for (int i = 0; i < N; i++)
        {
            sum += unaligned[i].c; // GRAB ONLY DOUBLE CUZ THAT WILL HAVE THE SHIFT GRABBING 8BYTES
        }
    }

    clock_t end = clock();

    printf("unaligned access time [ms]: %lf\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);
    free(unaligned);
}


int main(int argc, char *argv[])
{
    printf("%d\n", argc);
    if (argc > 1)
        unaligned();
    else
        aligned();

    return 0;

    // struct size: 16
    // struct alignment: 8
    // Benchmark: 
    // aligned access time [ms]: 18357.520000
    // unaligned access time [ms]: 18392.629000
}
