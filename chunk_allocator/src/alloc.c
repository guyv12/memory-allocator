#include "alloc.h"
#include <unistd.h>
#include <string.h>


int
chcreate()
{
    chunk_head = sbrk(0);
    chunk_tail = NULL;
    
    return 0;
}


int
chdestroy()
{
    brk(chunk_head);
    chunk_head = NULL;
    chunk_tail = NULL;

    return 0;
}


void *
challoc(size_t __size)
{
    void *addr = sbrk(0);

    size_t actual_alloc_size = __size + sizeof(chunk_meta_t);
    chunk_meta_t meta = { __size, false, NULL };


    void *request = sbrk(actual_alloc_size);
    if (addr == (void *) -1) return NULL;

    memcpy(addr, &meta, sizeof(meta));

    return addr;
}


void *
aligned_challoc(size_t __alignment, size_t __size)
{

}


void
chfree(void *__ptr)
{
    ((chunk_meta_t *)__ptr)->free = true;
}
