#pragma once

#ifndef my_arena_h
#define my_arena_h


#include <stdlib.h>
#include <stdint.h>


//--------------- FLAGS ----------------

typedef enum ArenaFlags
{
    ARENA_GROW          = 1 << 0, // enable exponential growth
    ARENA_SIZE_ALIGN    = 1 << 1, // align capacity to the next power of 2
}
ArenaFlags;


//--------------- ARENA ----------------

typedef struct ArenaAllocator
{
    void *mem;         // pointer to start of the available memory block
    void *sp;          // pointer to the top of the arena

    size_t size;       // size of the data allocated on the arena
    size_t capacity;   // size of the arena's allocated memory region
    
    ArenaFlags flags;
}
ArenaAllocator;


/* Initializes an arena metadata struct, allocates mmap() syscall */
int 
arcreate(ArenaAllocator *restrict __arena, const size_t __size, ArenaFlags __flags);

/* Destroys an arena, reclaiming allocated region back to the kernel. */
int
ardestroy(ArenaAllocator *restrict const __arena);

/* Allocates new object on the arena. */
void *
aralloc(ArenaAllocator *restrict const __arena, size_t __size);

/* Allocates new object on the arena, and aligns the pointer. */
void *
aligned_aralloc(ArenaAllocator *restrict const __arena, size_t __alignment, size_t __size);

/* Marks a new sub-arena beggining. */
size_t
armark(ArenaAllocator *restrict const __arena);

/* Rollbacks to the previous mark point, removing the sub-arena. */
int
arrollback(ArenaAllocator *restrict const __arena, size_t __mark_point);


//-------------- HELPERS ---------------

size_t 
next2_power(size_t x);

int
grow_arena(ArenaAllocator *restrict const __arena, size_t __new_elem_size);

static inline void *
align_address(void *__addr, size_t __alignment)
{
    return (void *)(((uintptr_t)__addr + __alignment - 1) & ~(__alignment - 1)); 
}


#endif // my_arena_h