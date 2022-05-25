#ifndef PHBASE_CORE_H
#define PHBASE_CORE_H

#include <stddef.h>
#include <stdint.h>

/*
 * Statically determine the length of a static array.
 */
#define PHBASE_STATIC_ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

/*
 * Statically determine the length of a static string.
 */
#define PHBASE_STATIC_STRLEN(s) (sizeof("" s "") - 1)

/*
 * Declare a parameter unused.
*/
#define PHBASE_UNUSED_PARAMETER(p) ((void)p)

/*
 * Determine if allocating count many items of size size causes an overflow
 * of size_t.
 */
static inline int
phbase_array_allocates(size_t count, size_t size)
{
    /* 
     * As a fast path overflow check, we check size and count are both
     * less than the square root of SIZE_MAX + 1. SIZE_MAX + 1 is
     * 2^(sizeof(size_t) * 8) (8 bits to a byte), so the square root of
     * this has half the exponent.
     */
    static const size_t SQRT_SIZE_OFLOW = 1UL << (sizeof(size_t) * 4);
    return ((count < SQRT_SIZE_OFLOW) && (size < SQRT_SIZE_OFLOW))
	|| !(SIZE_MAX / count < size);
}

#endif
