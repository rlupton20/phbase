#ifndef PHBASE_ALLOCATION_H
#define PHBASE_ALLOCATION_H

#include <stddef.h>
#include <stdint.h>

/******************************************************************************/
/* Determine if an array of count elements of size size can be allocated.     */
/*                                                                            */
/* Protects against unsigned overflow.                                        */
/******************************************************************************/
static inline int
array_allocates(size_t count, size_t size)
{
    /* As a fast path overflow check, we check size and count are both
       less than the square root of SIZE_MAX + 1. SIZE_MAX + 1 is
       2^(sizeof(size_t) * 8) (8 bits to a byte), so the square root of
       this has half the exponent. */
    static const size_t SQRT_SIZE_OFLOW = 1UL << (sizeof(size_t) * 4);
    return ((count < SQRT_SIZE_OFLOW) && (size < SQRT_SIZE_OFLOW))
	|| !(SIZE_MAX / count < size);
}

#endif
