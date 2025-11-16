#include "arena.h"

#include <sys/mman.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>


int InitArena(MyArena *restrict __arena, const size_t __size)
{
    size_t actual_capacity = next2_power(__size);

    __arena->_mem = mmap(NULL, actual_capacity, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (__arena->_mem == MAP_FAILED)
        return ARENA_ERROR;

    __arena->_sp = __arena->_mem;
    __arena->_size = 0; __arena->_capacity = actual_capacity;

    return ARENA_SUCCESS;
}


int FreeArena(MyArena *const __arena)
{
    munmap(__arena->_mem, __arena->_capacity);
    return ARENA_SUCCESS;
}


void *ArenaAllocate(MyArena *const __arena, size_t __size)
{
    if (__arena->_size + __size > __arena->_capacity) 
    {
        size_t old_capacity = __arena->_capacity;
        void *new_mapping = mmap(NULL, __arena->_capacity *= 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
        if (new_mapping == MAP_FAILED) return ARENA_FAIL;

        memcpy(new_mapping, __arena->_mem, __arena->_size);
        munmap(__arena->_mem, old_capacity); // deactivate old
        __arena->_mem = new_mapping; __arena->_sp = __arena->_mem;
    }

    __arena->_size += __size; __arena->_sp = ((uint8_t *)__arena->_mem + __arena->_size); 
    return __arena->_sp; // return the address of the allocated part
}


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
