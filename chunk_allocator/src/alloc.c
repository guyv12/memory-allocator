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



//--------- CHUNK METADATA -------------

bool
adjacent(mem_chunk_free_t *__left_chunk, mem_chunk_free_t *__right_chunk)
{
    return
    (
        (uintptr_t)__left_chunk + sizeof(mem_chunk_t) + payload_size(&__left_chunk->meta)
        ==
        (uintptr_t)__right_chunk
    );
}


void
merge_free_chunks(mem_chunk_free_t *__left_chunk, mem_chunk_free_t *__right_chunk)
{      
    __left_chunk->meta._payload_size = payload_size(&__left_chunk->meta) +
                                       payload_size(&__right_chunk->meta) +
                                       sizeof(mem_chunk_free_t); // the mid-header
    __left_chunk->meta._payload_size |= (__right_chunk->meta._payload_size & 7); // keep the flags (both SHOULD have the same - so or with this untouched)
    
    __left_chunk->fd = __right_chunk->fd;
    if (__right_chunk->fd)
        __right_chunk->fd->bk = __left_chunk;
}


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


mem_chunk_t *
find_free_chunk(size_t __size)
{
    mem_chunk_free_t *current, **head;
    if (tl_heap.main) head = &tl_heap.main->free_head;
    else if (tl_heap.dynamic) head = &tl_heap.dynamic->free_head;
    
    for (current = *head; current; current = current->fd)
    {
         if (payload_size(&current->meta) >= __size)
        {
            // get a small chunk from the big chunk
            if (payload_size(&current->meta) - __size >= MIN_CHUNK_PAYLOAD + sizeof(mem_chunk_t))
            {
                // build the small chunk
                mem_chunk_free_t *div_chunk = (mem_chunk_free_t *)((uint8_t *)current + __size);
                uint8_t flags = current->meta._payload_size & 7;
                div_chunk->meta._payload_size = payload_size(&current->meta) - __size | flags;

                if (current->bk)
                    current->bk->fd = div_chunk;

                if (current->fd)
                    current->fd->bk = div_chunk;

                div_chunk->bk = current->bk;
                div_chunk->fd = current->fd;

                current->meta._payload_size = __size | flags | 1; // retain flags and set in-use bit

                if (current == *head)
                    *head = div_chunk;
            }

            else
            {
                if (current->bk)
                    current->bk->fd = current->fd;

                if (current->fd)
                    current->fd->bk = current->bk;

                current->meta._payload_size |= 1; // set in-use

                if (current == *head)
                    *head = NULL;
            }

            #ifdef TLALLOC_DEBUG
                printf("FREE CHUNK REUSED\n");
            #endif

            return (mem_chunk_t *)current;
        }
    }

    #ifdef TLALLOC_DEBUG
        printf("SUFFICIENT FREE CHUNK NOT FOUND\n");
    #endif

    return NULL;
}


void
free_list_insert(mem_chunk_free_t *__freed_chunk)
{
    mem_chunk_free_t *next;
    if (tl_heap.main) 
    {
        next = tl_heap.main->free_head;
        if (!next) { tl_heap.main->free_head = __freed_chunk; return; }
    }
    else if (tl_heap.dynamic)
    {
        next = tl_heap.dynamic->free_head;
        if (!next) { tl_heap.dynamic->free_head = __freed_chunk; return; }
    }

    mem_chunk_free_t *prev = NULL;

    // I want to know that prev is the chunk on the left of __freed_chunk
    while((uintptr_t)next < (uintptr_t)__freed_chunk && next)
    {
        prev = next;
        next = next->fd;
    }

    // we know that bk exists cuz we resolve empty list elsewhere
    __freed_chunk->bk = prev;
    __freed_chunk->fd = next;

    if (prev)
    {
        prev->fd = __freed_chunk;
        if (adjacent(prev, __freed_chunk)) merge_free_chunks(prev, __freed_chunk);
    }

    if (next)
    {
        next->bk = __freed_chunk;
        if (adjacent(__freed_chunk, next)) merge_free_chunks(__freed_chunk, next);
    }
}


#ifdef TLALLOC_DEBUG
    void
    print_free_list()
    {
        mem_chunk_free_t *current, *head;
        if (tl_heap.main) head = tl_heap.main->free_head;
        else if (tl_heap.dynamic) head = tl_heap.dynamic->free_head;

        for (current = head; current; current = current->fd)
            printf("[chunk] size: %d, fd: %p, bk: %p\n", payload_size(&current->meta), current->fd, current->bk);
    }
#endif


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
        // first search the free list
        void *reused = find_free_chunk(__size);
        if (reused) return payload(reused);

        // fallback to default behavior
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
        // first search the free list
        void *reused = find_free_chunk(__size);
        if (reused) return payload(reused);

        // fallback to default behavior
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
    free_chunk->bk = NULL; free_chunk->fd = NULL; // zero out the pointer fields
    free_list_insert(free_chunk);

    #ifdef TLALLOC_DEBUG
        printf("FREED HEAP CHUNK\n");
    #endif
}