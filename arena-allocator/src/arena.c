#define _GNU_SOURCE

#include "arena.h"

#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>

#if defined(__linux__) && defined(__GLIBC__)
#define MREMAP_AVAIL 1
#endif


//--------------- ARENA ----------------

int
arcreate(ArenaAllocator *restrict __arena, const size_t __size, ArenaFlags __flags)
/* @Return  0 on success, -1 on failure */
{
    __arena->flags = __flags;

    size_t alloc_size = (__arena->flags & ARENA_SIZE_ALIGN) ? next2_power(__size) : __size;

    __arena->mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (__arena->mem == MAP_FAILED)
        return -1;

    __arena->sp = __arena->mem;
    __arena->size = 0; __arena->capacity = alloc_size;

    return 0;
}


int
ardestroy(ArenaAllocator *restrict const __arena)
/* @Return 0 on success -1 on failure */
{
    munmap(__arena->mem, __arena->capacity);
    return 0;
}


void *
aralloc(ArenaAllocator *restrict const __arena, size_t __size)
/* @Return pointer to the allocated mem region on success, NULL on failure */
{
    if (__arena->size + __size > __arena->capacity) 
    {
        if (!(__arena->flags & ARENA_GROW)) return NULL;

        grow_arena(__arena, __size);
    }

    void *addr = __arena->sp;
    __arena->size += __size;
    __arena->sp = ((uint8_t *)__arena->mem + __arena->size);

    return addr;
}


void *
aligned_aralloc(ArenaAllocator *restrict const __arena, size_t __alignment, size_t __size)
/* @Return aligned pointer to the allocated mem region on success, NULL on failure */
{
    void *aligned_addr = align_address(__arena->sp, __alignment);
    size_t padding = (uintptr_t)aligned_addr - (uintptr_t)__arena->sp;

    if (__arena->size + __size + padding > __arena->capacity) 
    {
        if (!(__arena->flags & ARENA_GROW)) return NULL;

        grow_arena(__arena, __size + padding);
        aligned_addr = align_address(__arena->sp, __alignment); // growth can move the ptrs
    }

    __arena->size += __size + padding;
    __arena->sp = ((uint8_t *)__arena->mem + __arena->size);

    return aligned_addr;
}


size_t
armark(ArenaAllocator *restrict const __arena)
{
    return (uint8_t *)__arena->sp - (uint8_t *)__arena->mem;
}


int
arrollback(ArenaAllocator *restrict const __arena, size_t __mark_point)
/* @Return 0 on success, -1 on failure */
{
    if (__arena->capacity < __mark_point)
        return -1; // mark_point not in the arena pool
    
    __arena->sp = (uint8_t *)__arena->mem + __mark_point;
    __arena->size = __mark_point;
    return 0;
}


//-------------- HELPERS ---------------

size_t
next2_power(size_t x)
{
    if (x <= 1) return 1;
    
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    
    #if SIZE_MAX > 0xFF
        x |= x >> 8;
    #endif
    #if SIZE_MAX > 0xFFFF
        x |= x >> 16;
    #endif
    #if SIZE_MAX > 0xFFFFFFFF
        x |= x >> 32;
    #endif

    return ++x;
}


int
grow_arena(ArenaAllocator *restrict const __arena, size_t __new_elem_size)
/* @Return 0 on success -1 on failure */
{
    size_t new_capacity = next2_power(__arena->capacity + __new_elem_size);

    #ifdef MREMAP_AVAIL
        void *new_mapping = mremap(__arena->mem, __arena->capacity, new_capacity, MREMAP_MAYMOVE);
        if (new_mapping == MAP_FAILED) return -1;
    #else
        size_t old_capacity = __arena->capacity;    
        void *new_mapping = mmap(NULL, new_capacity PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (new_mapping == MAP_FAILED) return -1;
        memcpy(new_mapping, __arena->mem, __arena->size);
        munmap(__arena->mem, old_capacity); // deactivate old
    #endif

    __arena->capacity = new_capacity;

    __arena->mem = new_mapping;
    __arena->sp = ((uint8_t *)__arena->mem + __arena->size);
    return 0;
}
