#pragma once

#ifndef my_allocator_h
#define my_allocator_h


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


//-------------- CHUNKS ----------------

typedef struct chunk_meta_t
{
    size_t size;
    bool free;
   
    chunk_meta_t *next;
}
chunk_meta_t;


static chunk_meta_t *chunk_head = NULL;
static chunk_meta_t *chunk_tail = NULL;


int
chcreate();

int
chdestroy();

void *
challoc(size_t __size);

void *
aligned_challoc(size_t __alignment, size_t __size);

void
chfree(void *__ptr);


//-------------- HELPERS ---------------

static inline void *
align_address(void *__addr, size_t __alignment)
{
    return (void *)(((uintptr_t)__addr + __alignment - 1) & ~(__alignment - 1)); 
}


#endif // my_allocator_h