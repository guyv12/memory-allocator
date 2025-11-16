#pragma once

#ifndef my_arena_h
#define my_arena_h


#include <stdlib.h>

#define ARENA_SUCCESS (0)                 // <-- success bin signal
#define ARENA_ERROR (-1)                  // <-- fail bin signal
#define ARENA_FAIL ((void *)-1)           // <-- failed pointer value

#define ARENA_SIZE_THRESHOLD (128 * 1024) // <-- when to change from brk() to mmap()

typedef struct MyArena MyArena;

struct MyArena
{
    void *_mem;         // pointer to start of the available memory block
    void *_sp;          // pointer to the top of the arena

    size_t _size;       // current arena size 
    size_t _capacity;   // current capacity - to grow exponentially like a vector would
};


int InitArena(MyArena *restrict __arena, const size_t __size);

int FreeArena(MyArena *__arena);

void *ArenaAllocate(MyArena *const __arena, size_t __size);

// -- helper --
size_t next2_power(size_t x);

#endif // my_arena_h