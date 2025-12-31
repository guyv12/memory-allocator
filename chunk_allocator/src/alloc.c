#include "alloc.h"

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>


//--------------- ARENA ----------------

int
init_arena(mem_arena_t *__arena)
{
    pthread_mutex_init(&__arena->lock, NULL);

    return 0;
}

int
destroy_arena(mem_arena_t *__arena)
{
    pthread_mutex_destroy(&__arena->lock);

    return 0;
}


//----------- DYNAMIC HEAP -------------

mem_heap_dynamic_t *
new_dynamic_heap(mem_heap_dynamic_t *const __prev_heap, size_t __mem_size)
{
    mem_heap_dynamic_t *new_heap =  mmap(NULL, sizeof(mem_heap_dynamic_t) + __mem_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (new_heap == MAP_FAILED) return NULL;

    new_heap->ar_ptr = __prev_heap->ar_ptr;
    new_heap->prev = __prev_heap;
    new_heap->size = __mem_size;
    return new_heap;
}

int
delete_dynamic_heap(mem_heap_dynamic_t *__heap)
{
    munmap(__heap, sizeof(mem_heap_dynamic_t) + __heap->size);
    return 0;
}


//-------------- ALLOCS ----------------


void *
tlalloc(size_t __size)
{
    if (__size > BRK_THRESHOLD)
    {
        void *chunk = mmap(NULL, sizeof(mem_chunk_t) + __size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (chunk == MAP_FAILED) return NULL;
        return payload(chunk);
    }
}

void
tlfree(void *__ptr)
{
    mem_chunk_t *chunk_meta = (mem_chunk_t *)(__ptr) - 1; // look back for metadata
    
    // we can even check for double free?

    if (mmaped(chunk_meta))
    {
        munmap(__ptr, sizeof(mem_chunk_t) + size(chunk_meta));
    }

    // remember to change in_use bit and cast to mem_chunk_free_t with set pointers

}