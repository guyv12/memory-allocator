#pragma once

#ifndef my_allocator_h
#define my_allocator_h


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


//--------- CHUNK METADATA -------------

typedef struct mem_chunk_t
{
    uint32_t _size; // all chunks will be aligned to 8 -> 3LSb's can hold data
}
mem_chunk_t;

static __always_inline void *
payload(mem_chunk_t *__metadata) { return (void *)(__metadata + 1); } // the memory after the chunk metadata


typedef struct mem_chunk_free_t
{
    mem_chunk_t meta;

    mem_chunk_free_t *fd; // add pointers to the free list
    mem_chunk_free_t *bk;
}
mem_chunk_free_t;

// Since we keep 3 lsbs for flags we need to ignore them here
static __always_inline uint32_t
size(mem_chunk_t *__metadata) { return __metadata->_size & ~7; }

// Flags
static __always_inline bool
allocated_arena(mem_chunk_t *__metadata) { return __metadata->_size & ((uint32_t)4); }

static __always_inline bool
mmaped(mem_chunk_t *__metadata) { return __metadata->_size & ((uint32_t)2); }

static __always_inline bool
in_use(mem_chunk_t *__metadata) { return __metadata->_size & ((uint32_t)1); }


//--------------- ARENA ----------------

typedef struct mem_arena_t
{
    pthread_mutex_t lock;
    
    void *bins[];
}
mem_arena_t;

int
init_arena(mem_arena_t *__arena);

int
destroy_arena(mem_arena_t *__arena);

//----------- DYNAMIC HEAP -------------

typedef struct mem_heap_dynamic_t
{
    mem_arena_t *ar_ptr;

    size_t size;
}
mem_heap_dynamic_t;

static __always_inline void *
memory(mem_heap_dynamic_t *__heap) { return (void *)(__heap + 1); } // the memory after heap metadata

mem_heap_dynamic_t *
new_dynamic_heap(mem_heap_dynamic_t *const __prev_heap);

int
delete_dynamic_heap(mem_heap_dynamic_t *__heap);


//-----------  MAIN HEAP ---------------

typedef struct mem_heap_main_t
{
    void *base;
    void *top;

    mem_chunk_free_t *free_list_head;
}
mem_heap_main_t;


//-------------- HEAP -----------------

typedef union mem_heap_t
{
    mem_heap_dynamic_t dynamic;
    mem_heap_main_t main;
}
mem_heap_t;



//-------------- ALLOCS ----------------

__thread mem_arena_t tl_arena;
__thread mem_heap_t tl_heap;


#define BRK_THRESHOLD (128 * 1024)

void *
tlalloc(size_t __size);

void
tlfree(void *__ptr);


//-------------- HELPERS ---------------

static __always_inline void *
align_address(void *__addr, size_t __alignment)
{
    return (void *)(((uintptr_t)__addr + __alignment - 1) & ~(__alignment - 1)); 
}


#endif // my_allocator_h