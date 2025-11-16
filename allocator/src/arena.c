#include "arena.h"

#include <sys/mman.h>
#include <limits.h>
#include <stdint.h>


//--------------- ARENA ----------------

int InitArena(Arena *restrict __arena, const size_t __size, ArenaFlags __flags)
{
    __arena->flags = __flags;

    size_t alloc_size = (__arena->flags & ARENA_SIZE_ALIGN) ? next2_power(__size) : __size;
    __arena->_mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (__arena->_mem == MAP_FAILED)
        return ARENA_ERROR;

    __arena->_sp = __arena->_mem;
    __arena->_size = 0; __arena->_capacity = alloc_size;

    return ARENA_SUCCESS;
}

int FreeArena(Arena *const __arena)
{
    munmap(__arena->_mem, __arena->_capacity);
    return ARENA_SUCCESS;
}


void *ArenaAllocate(Arena *const __arena, size_t __size)
{
    if (__arena->_size + __size > __arena->_capacity) 
    {
        if (!(__arena->flags & ARENA_GROW)) return ARENA_FAIL;

        void *new_mapping = mremap(__arena->_mem, __arena->_capacity, __arena->_capacity *= 2, MREMAP_MAYMOVE);
        if (new_mapping == MAP_FAILED) return ARENA_FAIL;

        __arena->_mem = new_mapping; __arena->_sp = __arena->_mem;
    }

    __arena->_size += __size; __arena->_sp = ((uint8_t *)__arena->_mem + __arena->_size); 
    return __arena->_sp; // return the address of the allocated part
}


//------------- SUB ARENA --------------

void *ArenaMakeSub(Arena *const __arena, size_t __size)
{
    
}

int ArenaFreeSub()
{

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
