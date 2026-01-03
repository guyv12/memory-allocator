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


void *thread_func()
{
    double *arr = tlalloc(10 * sizeof(double));
    tlfree(arr);
    print_free_list();

    double *arr2 = tlalloc(20 * sizeof(double));
    print_free_list();

    double *arr3 = tlalloc(5 * sizeof(double));
    print_free_list();

    tlfree(arr2);
    print_free_list();

    tlfree(arr3);
    print_free_list();

    SampleStruct *my_struct = tlalloc(sizeof(SampleStruct));
    print_free_list();
    tlfree(my_struct);
    print_free_list();

    destroy_tl_heap();
    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t worker;
    pthread_create(&worker, NULL, thread_func, NULL);

    thread_func(NULL);

    pthread_join(worker, NULL);

    return 0;
}