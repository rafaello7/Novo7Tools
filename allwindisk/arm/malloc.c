#include <malloc.h>

/* Memory management: 4 bytes are added to every malloc request,
 * then the request size is rounded up to a power of two.
 * Offset in mem_area of a memory chunk of size 2^k is a multiple of 2^k.
 * First four bytes of allocated/free memory chunk contain the chunk
 * size divided by 4, plus 1 when the chunk is allocated.
 */

uint32_t *mem_area = NULL;
uint32_t mem_area_size = 0;

/* Assumes that start parameter value is at 4-byte boundary and
 * size is at least 8
 */
void malloc_init(void *start, uint32_t size)
{
    uint32_t *ptr;

    mem_area = start;
    size = size >> 2  &  ~1U;
    mem_area_size = size;
    ptr = mem_area;
    /* round down size to a power of two */
    while( size & (size - 1) )
        size &= size - 1;
    /* mark free chunk(s) */
    while(1) {
        *ptr = size;
        if( ptr - mem_area + size >= mem_area_size )
            return;
        ptr += size;
        while( ptr - mem_area + size > mem_area_size )
            size >>= 1;
    }
}

void *malloc(size_t size)
{
    uint32_t *ptr = mem_area;

    if( size == 0 )
        return NULL;
    size = (size+7) >> 2;
    /* round up size to a power of two */
    while( (size & (size - 1)) != 0 )
        size &= size - 1;
    size <<= 1;
    /* search for a free and enough big memory chunk */
    while( *ptr & 1 || *ptr < size ) {
        uint32_t chsize = *ptr & ~1U;
        if( chsize < size )
            chsize = size;
        if( ptr - mem_area + chsize >= mem_area_size )
            return NULL;
        ptr += chsize;
    }
    /* isolate exact size piece of the memory chunk */
    while( *ptr > size ) {
        *ptr >>= 1;
        ptr[*ptr] = *ptr;
    }
    /* mark the memory as allocated */
    ++*ptr;

    return ptr + 1;
}

void free(void *memptr)
{
    uint32_t *ptr = memptr;

    if( ptr == NULL )
        return;
    --ptr;
    /* mark the memory as unallocated */
    *ptr &= ~1U;
    /* merge chunks */
    while(1) {
        unsigned twin_offset = (ptr - mem_area) ^ *ptr;
        if( twin_offset >= mem_area_size )
            return;
        uint32_t *twin = mem_area + twin_offset;
        if( *twin != *ptr )
            return;
        if( twin < ptr )
            ptr = twin;
        *ptr <<= 1;
    }
}

