#ifndef PHBASE_BITARRAY_H
#define PHBASE_BITARRAY_H

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Determine the amount of memory needed for a bitarray containing
 * a number of bits.
 */
#define PHBASE_BITARRAY_MEMORY_SIZE(bits) ((size_t)((bits) / 8))

/*
 * How to align memory for bitarray memory.
 */
#define PHBASE_BITARRAY_MEMORY_ALIGN (alignof(uint8_t))

/*
 * Opaque structure type for bitarrays.
 */
struct phbase_bitarray
{
    uint8_t* memory;
};

/*
 * Create a bitarray from a block of memory. The user of the bitarray
 * is expected to deallocate the memory (if heap allocated) after the
 * bitarray has been used.
 */
static inline struct phbase_bitarray
phbase_bitarray_from_memory(void* memory)
{
    return (struct phbase_bitarray){
	.memory = memory
    };
}

/*
 * Set a bit in the bitarray.
 */
static inline void
phbase_bitarray_set_bit(const struct phbase_bitarray* ba, size_t bit)
{
    /*
     * The bottom 3 bits index a bit in a byte, the top bits index
     * the byte.
     */
    const size_t byte = bit >> 3;
    const uint8_t shift = bit & 7;

    ba->memory[byte] |= (1 << shift);
}

/*
 * Clear a bit in the bitarray.
 */
static inline void
phbase_bitarray_clear_bit(const struct phbase_bitarray* ba, size_t bit)
{
    /*
     * The bottom 3 bits index a bit in a byte, the top bits index
     * the byte.
     */
    const size_t byte = bit >> 3;
    const uint8_t shift = bit & 7;

    ba->memory[byte] &= ~(1 << shift);
}

/*
 * Test a bit in the bitarray.
 */
static inline int
phbase_bitarray_bit_is_set(const struct phbase_bitarray* ba, size_t bit)
{
    /*
     * The bottom 3 bits index a bit in a byte, the top bits index
     * the byte.
     */
    const size_t byte = bit >> 3;
    const uint8_t shift = bit & 7;

    return ba->memory[byte] & (1 << shift);
}

#endif
