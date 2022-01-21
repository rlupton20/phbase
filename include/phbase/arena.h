#ifndef PHBASE_ARENA_H
#define PHBASE_ARENA_H

#include <stdint.h>
#include <stddef.h>

/******************************************************************************/
/* Control block for an arena allocator.                                      */
/******************************************************************************/
struct phbase_arena
{
    size_t size;        /* The size of the current memory buffer.         */
    size_t current;     /* Offset to next free byte.                      */
    char* memory;       /* Pointer to the beginning of the memory buffer. */
};

/******************************************************************************/
/* Initialize an arena allocator with a block of memory.                      */
/******************************************************************************/
static inline void
phbase_arena_init(struct phbase_arena* a, size_t size, void* memory)
{
    a->size = size;
    a->current = 0;
    a->memory = memory;
}

/******************************************************************************/
/* Reset the allocator, throwing away all information of prior allocations.   */
/******************************************************************************/
static inline void
phbase_arena_reset(struct phbase_arena* a)
{
    a->current = 0;
}

/******************************************************************************/
/* Allocate an aligned chunk of memory. Returns NULL if the arena is full.    */
/******************************************************************************/
static inline void*
phbase_arena_allocate(struct phbase_arena* a, size_t align, size_t size)
{
    /* First we need to align the current offset. Note that a->memory    */
    /* may not be suitably aligned, so we can't just align a->current.   */

    /* Note that bits is equivalent to the more civilized                */
    /*   size_t bits = (uintptr_t)(a->memory + a->current) & (align - 1) */
    /* but the address sanitizer isn't that smart, and complains about   */
    /* overflow of pointers if we compute a->memory + a->current with    */
    /* some of our test data. Although the test data isn't realistic,    */
    /* pacify the address sanitizer for now.                             */
    size_t bits = (((uintptr_t)a->memory & (align - 1)) + (a->current & (align - 1))) & (align - 1);
    size_t offset = (a->current & ~(align - 1)) + (bits > 0) * align;

    /* Did aligning cause us to wrap? */
    if (offset < a->current)
    {
	return NULL;
    }

    /* Will the size cause us to wrap? */
    if (offset + size < offset)
    {
	return NULL;
    }

    /* Check there is space for this allocation. */
    if (offset + size > a->size)
    {
	return NULL;
    }

    a->current = offset + size;

    return a->memory + offset;
}

#endif
