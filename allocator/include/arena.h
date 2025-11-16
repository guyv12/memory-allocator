#pragma once

#ifndef my_arena_h
#define my_arena_h


#include <stdlib.h>

//--------------- CONSTS ---------------

#define ARENA_SUCCESS (0)                 // <-- success bin signal
#define ARENA_ERROR (-1)                  // <-- fail bin signal
#define ARENA_FAIL ((void *)-1)           // <-- failed pointer value

#define ARENA_SIZE_THRESHOLD (128 * 1024) // <-- when to change from brk() to mmap()


//--------------- FLAGS ----------------

typedef enum ArenaFlags ArenaFlags;

enum ArenaFlags
{
    ARENA_GROW          = 0,      // enable exponential growth
    ARENA_SIZE_ALIGN    = 1 << 0, // align capacity to the next power of 2
};


//--------------- ARENA ----------------

typedef struct Arena Arena;

struct Arena
{
    void *_mem;         // pointer to start of the available memory block
    void *_sp;          // pointer to the top of the arena

    size_t _size;       // current arena size 
    size_t _capacity;   // current capacity - to grow exponentially like a vector would
    
    ArenaFlags flags;
};

int InitArena(Arena *restrict __arena, const size_t __size, ArenaFlags __flags);

int FreeArena(Arena *__arena);

void *ArenaAllocate(Arena *const __arena, size_t __size);


//------------- SUB ARENA --------------

struct SubArena
{
    Arena *main_arena;

    void *_mem;
    void *_sp;
    size_t _size;
};

void *ArenaMakeSub(Arena *const __arena, size_t __size);

int ArenaFreeSub();

//-------------- HELPERS ---------------

size_t next2_power(size_t x);

#endif // my_arena_h