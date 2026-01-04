#include "alloc.h"
#include <stdio.h>
#include <pthread.h>


typedef struct SampleStruct
{
        int d;
        float f;
        char c[10];
}
SampleStruct;

const int n_structs = 10;

void *thread_func()
{
    long long unsigned int alloc_size = -1;
    void *unsuccessful = tlalloc(alloc_size);
    printf("%p\n", unsuccessful);


    destroy_tl_heap();
    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t worker;
    pthread_create(&worker, NULL, thread_func, NULL);

    pthread_join(worker, NULL);

    return 0;
}