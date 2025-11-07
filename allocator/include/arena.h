#pragma once

#ifndef my_arena_h
#define my_arena_h


#include <stdlib.h>


typedef struct MyArena MyArena;

struct MyArena
{
    void *mem;
    size_t sp;
};


void InitArena(MyArena *restrict arena, const size_t size);

void FreeArena(MyArena *arena);

void ArenaPush(MyArena *const arena, void *data);

#endif // my_arena_h