#ifndef PHBASE_ALLOCATOR_H
#define PHBASE_ALLOCATOR_H

#include <stddef.h>

/*
 * malloc, but with an explicit allocator parameter.
 */
#define PHBASE_ALLOCATOR_MALLOC(a, size)       ((a)->realloc((a), NULL, (size)))

/*
 * realloc, but with an explicit allocator parameter.
 */
#define PHBASE_ALLOCATOR_REALLOC(a, ptr, size) ((a)->realloc((a), (ptr), (size)))

/*
 * free, but with an explicit allocator parameter.
 */
#define PHBASE_ALLOCATOR_FREE(a, ptr)          ((a)->realloc((a), (ptr), 0))

/*
 * Interface for allocation of memory.
 */
struct phbase_allocator
{
    void* (*realloc)(const struct phbase_allocator* a, void* ptr, size_t size);
};

/*
 * Standard libc procedures for allocation.
 */
extern const struct phbase_allocator phbase_allocator_libc;

#endif
