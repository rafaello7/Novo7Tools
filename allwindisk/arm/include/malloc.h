#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <types.h>

void malloc_init(void *start, uint32_t size);
void *malloc(size_t size);
void free(void *ptr);

#endif
