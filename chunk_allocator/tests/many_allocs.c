#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


typedef struct list list;
struct list
{
    void *self;
    list *prev;
};

const int N = 10000;

void *thread_func()
{
    long long unsigned int alloc_size = -1;
    void *unsuccessful = tlalloc(alloc_size);
    printf("%p\n", unsuccessful);

    list val, *head;
    head = &val;
    head->prev = NULL;
    head->self = NULL;

    for (int i = 0; i < N; i++)
    {
        if (i >= 9917)
        {
            print_free_list();
        }
        int result = rand() % 100;
        printf("i: %d choice: %d\n", i, result);
        if (result < 50)
        {
            list *next = tlalloc(sizeof(list));
            if (!next) { printf("NULL\n"); continue; }

            next->self = tlalloc(rand() % 256);
            next->prev = head;
            head = next;
        }

        else if (head && head != &val)
        {
            list *to_free = head;
            head = head->prev;
            
            if(to_free->self)
                tlfree(to_free->self);
            tlfree(to_free);
        }
    }

    destroy_tl_heap();
    return NULL;
}


int main(int argc, char *argv[])
{
    srand(1337);

    pthread_t worker;
    pthread_create(&worker, NULL, thread_func, NULL);

    pthread_join(worker, NULL);

    return 0;
}