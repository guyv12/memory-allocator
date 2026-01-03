#pragma once

#ifndef my_allocator_h
#define my_allocator_h

#define TLALLOC_DEBUG

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/syscall.h>


//--------- CHUNK METADATA -------------

typedef struct mem_chunk_t
{
    uint32_t _payload_size; // all chunks payload size will be a multiple of 8 -> 3LSb's can hold data
}
mem_chunk_t;

static __always_inline void *
payload(mem_chunk_t *__metadata) { return (void *)(__metadata + 1); } // the memory after the chunk metadata


typedef struct mem_chunk_free_t
{
    mem_chunk_t meta;

    struct mem_chunk_free_t *fd; // add pointers to the free list
    struct mem_chunk_free_t *bk;
}
mem_chunk_free_t;

#define MIN_CHUNK_PAYLOAD sizeof(mem_chunk_free_t) - sizeof(mem_chunk_t) // we need the chunk to be able to hold the free metadata after free

// Since we keep 3 lsbs for flags we need to ignore them here
static __always_inline uint32_t
payload_size(mem_chunk_t *__metadata) { return __metadata->_payload_size & (~7); }

// Flag retrieval
static __always_inline bool
allocated_arena(mem_chunk_t *__metadata) { return __metadata->_payload_size & ((uint32_t)4); }

static __always_inline bool
mmaped(mem_chunk_t *__metadata) { return __metadata->_payload_size & ((uint32_t)2); }

static __always_inline bool
in_use(mem_chunk_t *__metadata) { return __metadata->_payload_size & ((uint32_t)1); }

// Free list helpers / inserts
bool
adjacent(mem_chunk_free_t *__chunk1, mem_chunk_free_t *__chunk2);

void
merge_free_chunks(mem_chunk_free_t *__left_chunk, mem_chunk_free_t *__right_chunk);


//--------------- ARENA ----------------

typedef struct mem_arena_t // not used
{
    pthread_mutex_t lock;
    
    void *fastbins;
    void *bins;
}
mem_arena_t;

int
init_arena(mem_arena_t *__arena);

int
destroy_arena(mem_arena_t *__arena);

//----------- DYNAMIC HEAP -------------

typedef struct mem_heap_dynamic_t
{
    mem_arena_t *ar_ptr; // not used
    
    uint32_t payload_size;
    void *top;

    mem_chunk_free_t *free_head; // get rid of arena implemented?
}
mem_heap_dynamic_t;

#define DYNAMIC_HEAP_SIZE 64 * 1024 * 1024

static __always_inline void *
memory(mem_heap_dynamic_t *__heap) { return (void *)(__heap + 1); } // the memory after heap metadata

static __always_inline size_t
avail(mem_heap_dynamic_t *__heap) { return __heap->payload_size - (uintptr_t)__heap->top; }

mem_heap_dynamic_t *
new_dynamic_heap(mem_arena_t *const __arena);

int
delete_dynamic_heap(mem_heap_dynamic_t *__heap);


//-----------  MAIN HEAP ---------------

typedef struct mem_heap_main_t
{
    void *base;
    void *top;

    mem_chunk_free_t *free_head;
}
mem_heap_main_t;


//----------- HEAP INFO ---------------

typedef struct mem_heap_info_t
{
    mem_heap_dynamic_t *dynamic;
    mem_heap_main_t *main;
}
mem_heap_info_t;


//-------------- ALLOCS ----------------

extern __thread mem_heap_info_t tl_heap;

void
init_tl_heap();

void
destroy_tl_heap();

mem_chunk_t *
find_free_chunk(size_t __size);

void
free_list_insert(mem_chunk_free_t *__freed_chunk);

#ifdef TLALLOC_DEBUG
    void
    print_free_list();
#endif


#define BRK_THRESHOLD (128 * 1024)


void *
tlalloc(size_t __size);

void
tlfree(void *__ptr);


//-------------- HELPERS ---------------

#define gettid() syscall(SYS_gettid)


static __always_inline size_t
align8(size_t __number)
{
    return (__number + 7) & (~7);
}


static __always_inline void *
align_address(void *__addr, size_t __alignment)
{
    return (void *)(((uintptr_t)__addr + __alignment - 1) & ~(__alignment - 1)); 
}


#endif // my_allocator_h