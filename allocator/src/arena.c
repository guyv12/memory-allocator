#include "arena.h"

#include <sys/mman.h>


void InitArena(MyArena *restrict arena, const size_t size)
{
    arena->mem = mmap();
    arena->sp = 0;
}


void FreeArena(MyArena *const arena)
{

}


void ArenaPush(MyArena *const arena, void *data)
{
    arena->sp -= 0;
}

