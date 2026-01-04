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
    double *arr = tlalloc(2 * sizeof(double));
    arr[0] = 0.05; arr[1] = 0.3;

    printf("arr: %lf, %lf\n", arr[0], arr[1]);

    SampleStruct *my_struct = tlalloc(sizeof(SampleStruct));
    my_struct->d = 10; my_struct->f = 0.5; my_struct->c[0] = 'a';
    printf("%d, %f, %s\n", my_struct->d, my_struct->f, my_struct->c);

    tlfree(arr);
    tlfree(my_struct);

    const int n_structs = 10;
    SampleStruct **structs = tlalloc(n_structs * sizeof(SampleStruct *));
    for (int i = 0; i < n_structs; i++)
        structs[i] = tlalloc(n_structs * sizeof(SampleStruct));

    for (int i = 0; i < n_structs; i++)
    {
        for (int j = 0; j < n_structs; j++)
        {
            structs[i][j].d = i;
            structs[i][j].f = i + 0.05;
            structs[i][j].c[0] = 'a';
        
            printf("%d, %f, %s\n", structs[i][j].d, structs[i][j].f, structs[i][j].c);
        }
        printf("\n");
    }

    tlfree(*structs);
    tlfree(structs);

    destroy_tl_heap();
    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t worker;
    pthread_create(&worker, NULL, thread_func, NULL);

    thread_func(NULL);

    pthread_join(worker, NULL);

    int *p = tlalloc(50 * 1024 * sizeof(int)); // large alloc
    
    p[50 * 1023] = 123;
    printf("%d\n", p[50 * 1023]);
    
    tlfree(p);

    return 0;
}