#define TLALLOC_DEBUG

#include "alloc.h"

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>

#ifdef TLALLOC_DEBUG
    #include <stdio.h>
#endif

__thread mem_heap_info_t tl_heap;


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
new_dynamic_heap(mem_arena_t *const __arena)
{
    mem_heap_dynamic_t *new_heap =  mmap(NULL, sizeof(mem_heap_dynamic_t) + DYNAMIC_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (new_heap == MAP_FAILED) return NULL;

    new_heap->ar_ptr = __arena;

    new_heap->payload_size = DYNAMIC_HEAP_SIZE;
    new_heap->top = memory(new_heap);

    new_heap->free_head = NULL;
    new_heap->free_tail = NULL;

    #ifdef TLALLOC_DEBUG
        printf("CREATED DYNAMIC HEAP\n");
    #endif

    return new_heap;
}

int
delete_dynamic_heap(mem_heap_dynamic_t *__heap)
{
    munmap(__heap, sizeof(mem_heap_dynamic_t) + __heap->payload_size);

    #ifdef TLALLOC_DEBUG
        printf("DELETED DYNAMIC HEAP\n");
    #endif

    return 0;
}


//-------------- ALLOCS ----------------

void
init_tl_heap()
{
    pid_t tid = gettid();
    pid_t pid = getpid();

    if (tid == pid) // main thread
    {
        static mem_heap_main_t main_heap_meta;
        main_heap_meta.base = sbrk(0);
        main_heap_meta.top = main_heap_meta.base;

        main_heap_meta.free_head = NULL;
        main_heap_meta.free_tail = NULL;

        tl_heap.main = &main_heap_meta;
        tl_heap.dynamic = NULL;
    }

    else
    {
        tl_heap.dynamic = new_dynamic_heap(NULL);
        tl_heap.main = NULL;
    }

    #ifdef TLALLOC_DEBUG
        printf("INITIALIZED THREAD LOCAL HEAP\n");
    #endif
}


void
destroy_tl_heap()
{
    if (tl_heap.dynamic)
    {
        delete_dynamic_heap(tl_heap.dynamic);
    }

    #ifdef TLALLOC_DEBUG
        printf("DESTROYED THREAD LOCAL HEAP\n");
    #endif
}


void *
tlalloc(size_t __size)
{
    if (tl_heap.main == NULL && tl_heap.dynamic == NULL)
        init_tl_heap();

    // we need to align size to divisible by 8 to store flags and extend to min payload size
    __size = align8(__size < MIN_CHUNK_PAYLOAD ? MIN_CHUNK_PAYLOAD : __size);

    mem_chunk_t *chunk = NULL;
    size_t total_alloc = sizeof(mem_chunk_t) + __size;
    if (__size > BRK_THRESHOLD)
    {
        chunk = mmap(NULL, total_alloc, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (chunk == MAP_FAILED) return NULL;

        chunk->_payload_size = __size | 2 | 1; // mmaped and in_use flags

        #ifdef TLALLOC_DEBUG
            printf("ALLOCATED MMAPED CHUNK\n");
        #endif
    }

    else if (tl_heap.main)
    {
        // first search free list ...

        chunk = sbrk(total_alloc);
        if (chunk == (void *)-1) return NULL;

        tl_heap.main->top = (uint8_t *)chunk + total_alloc;

        chunk->_payload_size = __size | 1; // in_use flag

        #ifdef TLALLOC_DEBUG
            printf("ALLOCATED MAIN HEAP CHUNK\n");
        #endif
    }

    else if (tl_heap.dynamic)
    {
        // first search free list ...
        
        if (avail(tl_heap.dynamic) < __size) return NULL;

        chunk = (mem_chunk_t *)tl_heap.dynamic->top;

        tl_heap.dynamic->top = (uint8_t *)chunk + total_alloc;

        chunk->_payload_size = __size | 4 | 1; // arena and in_use flags

        #ifdef TLALLOC_DEBUG
            printf("ALLOCATED DYNAMIC HEAP CHUNK\n");
        #endif
    }

    return chunk ? payload(chunk) : NULL;
}

void
tlfree(void *__ptr)
{
    mem_chunk_t *chunk_meta = (mem_chunk_t *)(__ptr) - 1; // look back for metadata

    if (mmaped(chunk_meta))
    {
        munmap(chunk_meta, sizeof(mem_chunk_t) + payload_size(chunk_meta));

        #ifdef TLALLOC_DEBUG
            printf("FREED MMAPED CHUNK\n");
        #endif

        return;
    }

    mem_chunk_free_t *free_chunk = (mem_chunk_free_t *)chunk_meta;
    free_chunk->meta._payload_size &= ((uint32_t)-2); // zero out the in_use bit

    if (!allocated_arena(chunk_meta))
    {
        free_chunk->fd = tl_heap.main->free_head;
        free_chunk->bk = tl_heap.main->free_tail;
        
        tl_heap.main->free_tail = free_chunk;

        if (free_chunk->bk)
            free_chunk->bk->fd = free_chunk;

        #ifdef TLALLOC_DEBUG
            printf("FREED MAIN HEAP CHUNK\n");
        #endif
    }

    else
    {
        free_chunk->fd = tl_heap.dynamic->free_head;
        free_chunk->bk = tl_heap.dynamic->free_tail;

        tl_heap.dynamic->free_tail = free_chunk;

        if (free_chunk->bk)
            free_chunk->bk->fd = free_chunk;
        
        #ifdef TLALLOC_DEBUG
            printf("FREED DYNAMIC HEAP CHUNK\n");
        #endif
    }
}