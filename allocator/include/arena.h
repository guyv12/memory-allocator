#pragma once

#ifndef my_arena_h
#define my_arena_h


#include <stdlib.h>

#define ARENA_SUCCESS 0
#define ARENA_ERROR -1

typedef struct MyArena MyArena;

struct MyArena
{
    void *_mem;
    void *_sp;

    size_t _size;
    size_t _capacity;
};


int InitArena(MyArena *restrict __arena, const size_t __size);

int FreeArena(MyArena *__arena);

int ArenaPush(MyArena *const __arena, void *__data, size_t __size);

#endif // my_arena_h