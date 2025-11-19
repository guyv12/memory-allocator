#define _GNU_SOURCE

#include "arena.h"

#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#if defined(__linux__) && defined(__GLIBC__)
#define MREMAP_AVAIL 1
#endif


//--------------- ARENA ----------------

int arcreate(ArenaAllocator *restrict __arena, const size_t __size, ArenaFlags __flags)
/* @Return  0 on success, -1 on failure */
{
    __arena->flags = __flags;

    size_t alloc_size = (__arena->flags & ARENA_SIZE_ALIGN) ? next2_power(__size) : __size;

    __arena->mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (__arena->mem == MAP_FAILED)
        return -1;

    __arena->sp = __arena->mem;
    __arena->size = 0; __arena->capacity = alloc_size;

    return 0;
}

int ardestroy(ArenaAllocator *const __arena)
/* @Return 0 on success -1 on failure */
{
    munmap(__arena->mem, __arena->capacity);
    return 0;
}


void *aralloc(ArenaAllocator *const __arena, size_t __size)
/* @Return pointer to the allocated mem region on success, NULL on failure */
{
    if (__arena->size + __size > __arena->capacity) 
    {
        if (!(__arena->flags & ARENA_GROW)) return NULL;

        #ifdef MREMAP_AVAIL
            void *new_mapping = mremap(__arena->mem, __arena->capacity, __arena->capacity *= 2, MREMAP_MAYMOVE);
            if (new_mapping == MAP_FAILED) return NULL;
        #else
            size_t old_capacity = __arena->capacity;    
            void *new_mapping = mmap(NULL, __arena->capacity *= 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
            if (new_mapping == MAP_FAILED) return NULL;
            memcpy(new_mapping, __arena->mem, __arena->size);
            munmap(__arena->mem, old_capacity); // deactivate old
        #endif

        __arena->mem = new_mapping; __arena->sp = __arena->mem;
    }

    void *addr = __arena->sp;
    __arena->size += __size; __arena->sp = ((uint8_t *)__arena->mem + __arena->size);

    return addr;
}


size_t armark(ArenaAllocator *const __arena)
{
    return (uint8_t *)__arena->sp - (uint8_t *)__arena->mem;
}


int arrollback(ArenaAllocator *const __arena, size_t __mark_point)
/* @Return 0 on success, -1 on failure */
{
    if (__arena->capacity < __mark_point)
        return -1; // mark_point not in the arena pool
    
    __arena->sp = (uint8_t *)__arena->mem + __mark_point;
    __arena->size = __mark_point;
    return 0;
}


//-------------- HELPERS ---------------

size_t next2_power(size_t x)
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
