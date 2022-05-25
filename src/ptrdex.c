#include <phbase/ptrdex.h>

#include <phbase/allocator.h>
#include <phbase/status.h>

#include "allocation.h"

#include <stdalign.h>

#include <assert.h> /* assert */
#include <string.h> /* memset */

#define PHBASE_ASSERT assert
#define PHBASE_MEMCPY memcpy
#define PHBASE_MEMSET memset

#define MIN_CAPACITY ((size_t)16)
#define LOAD_FACTOR  ((float)0.7)
#define KEY_SIZE     sizeof(void*)

#define U32_HIGH_BIT ((uint32_t)1 << 31)
#define PRESENT_BIT  (U32_HIGH_BIT)
#define HASH_MASK    (~PRESENT_BIT)

static inline size_t
alignup(size_t count)
{
    static const size_t align = alignof(max_align_t);
    return (count + align - 1) & ~(align - 1);
}

int
phbase_ptrdex__init_with_capacity(struct phbase_ptrdex__internal* t, size_t capacity,
				  size_t value_size)
{
    capacity = capacity < MIN_CAPACITY ? MIN_CAPACITY : capacity;

    /* Note we only hash to 32-bits, and then truncate this to 31-bits, so
     * there is no point inserting more than HASH_MASK items.
     * TODO Allow inserting more that HASH_MASK items. */
    if (capacity > HASH_MASK)
    {
	return PHBASE_STATUS_INVALID_PARAMETER;
    }

    size_t value_offset = alignup(KEY_SIZE * capacity);
    size_t locator_offset = value_offset + alignup(value_size * capacity);
    size_t allocation =  locator_offset + (sizeof(t->locator[0]) * capacity);

    /* Check each array allocates, and then check that neither the
     * addition overflows, nor the alignup. Note that if the alignup
     * overflows, then because alignof(max_align_t) is a power of
     * two, and so divides SIZE_MAX + 1, the alignup overflows to
     * zero. So given keys and values and locators all have non-zero
     * size, we can check both alignup and addition overflow by
     * checking our offsets are strictly increasing.*/
    PHBASE_ASSERT(value_size > 0);
    /* NOTE KEY_SIZE > 0 */

    if (!array_allocates(capacity, KEY_SIZE) ||
	!array_allocates(capacity, value_size) ||
	!array_allocates(capacity, sizeof(t->locator[0])) ||
	value_offset < KEY_SIZE * capacity ||
	locator_offset <= value_offset ||
	allocation <= locator_offset)
    {
	return PHBASE_STATUS_NO_MEMORY;
    }

    uint8_t* mem = PHBASE_ALLOCATOR_MALLOC(&phbase_allocator_libc, allocation);

    if (!mem)
    {
	return PHBASE_STATUS_NO_MEMORY;
    }

    PHBASE_MEMSET(mem, 0, allocation);

    t->key = (void*)mem;
    t->value = (void*)(mem + value_offset);
    t->locator = (void*)(mem + locator_offset);
    t->capacity = capacity;
    t->used = 0;

    return PHBASE_STATUS_OK;
}

void
phbase_ptrdex__clear(struct phbase_ptrdex__internal* t)
{
    PHBASE_ALLOCATOR_FREE(&phbase_allocator_libc, t->key);
    *t = (struct phbase_ptrdex__internal){ 0 };
}

int
phbase_ptrdex__is_initialized(const struct phbase_ptrdex__internal* t)
{
    return (t->key != NULL);
}

static inline uint32_t
pointer_hash32(void* ptr)
{
    uint64_t h = (uintptr_t)ptr;
    h = (~h) + (h << 18);
    h = h ^ (h >> 31);
    h = h * 21;
    h = h ^ (h >> 11);
    h = h + (h << 6);
    h = h ^ (h >> 22);
    return ((uint32_t)h) & HASH_MASK;
}

int
phbase_ptrdex__find(const struct phbase_ptrdex__internal* t, void* key, size_t* out)
{
    PHBASE_ASSERT(key);

    void** keys = t->key;

    const size_t mask = t->capacity - 1;
    const uint32_t hash = pointer_hash32(key);
    size_t slot = hash & mask;

    while (t->locator[slot] & PRESENT_BIT &&
	   ((t->locator[slot] & HASH_MASK) != hash || keys[slot] != key))
    {
	slot = (slot + 1) & mask;
    }

    if (keys[slot])
    {
	PHBASE_ASSERT(t->locator[slot] & PRESENT_BIT);
	PHBASE_ASSERT((t->locator[slot] & HASH_MASK) == hash);
	PHBASE_ASSERT(keys[slot] == key);

	*out = slot;
	return PHBASE_STATUS_OK;
    }

    PHBASE_ASSERT(!(t->locator[slot] & PRESENT_BIT));
    PHBASE_ASSERT(!keys[slot]);

    return PHBASE_STATUS_DOES_NOT_EXIST;
}

static void
repack(struct phbase_ptrdex__internal* restrict into, const struct phbase_ptrdex__internal* from,
       size_t value_size)
{
    const size_t mask = into->capacity - 1;

    void** keys = from->key;
    void** newkeys = into->key;

    /* TODO This assertion is way too strong. */
    PHBASE_ASSERT(into->capacity >= from->capacity);
    PHBASE_ASSERT(into->used == 0);

    /* Copy across old data - note we only visit each key once, and
     * our new table starts empty, so we only need to check the
     * presence bit when looking for a slot. */
    for (size_t ix = 0; ix < from->capacity; ++ix)
    {
	if (keys[ix])
	{
	    size_t slot = (from->locator[ix] & HASH_MASK) & mask;

	    while (into->locator[slot] & PRESENT_BIT)
	    {
		slot = (slot + 1) & mask;
	    }

	    PHBASE_ASSERT(!newkeys[slot]);
	    into->locator[slot] = from->locator[ix];
	    newkeys[slot] = keys[ix];
	    PHBASE_MEMCPY((char*)into->value + (slot * value_size),
			  (char*)from->value + (ix * value_size),
			  value_size);
	}
    }

    into->used = from->used;
}

int
phbase_ptrdex__obtain_slot(struct phbase_ptrdex__internal* t, void* key,
			   struct phbase_ptrdex_locator* out, size_t value_size)
{
    PHBASE_ASSERT(key != NULL);

    void** keys = t->key;

    const size_t mask = (t->capacity) - 1;
    const uint32_t hash = pointer_hash32(key);

    /* Scan slots starting at the masked hash until we find either one
     * free, or one matching the key. */
    uint32_t ix = hash & mask;
    while (t->locator[ix] & PRESENT_BIT &&
	   ((t->locator[ix] & HASH_MASK) != hash || keys[ix] != key))
    {
	ix = (ix + 1) & mask;
    }

    PHBASE_ASSERT(ix < t->capacity);

    /* We have two cases, either keys[ix] is key, in which case we
     * have already recorded this key, or else weC'll need to occupy
     * a currently empty slot. */
    if (t->locator[ix] & PRESENT_BIT)
    {
	PHBASE_ASSERT((t->locator[ix] & HASH_MASK) == hash);
	PHBASE_ASSERT(keys[ix] == key);

	*out = (struct phbase_ptrdex_locator){
	    .locator = PRESENT_BIT | hash,
	    .slot = ix,
	};

	return PHBASE_STATUS_OK;
    }

    PHBASE_ASSERT(!(t->locator[ix] & PRESENT_BIT));
    PHBASE_ASSERT(!keys[ix]);

    /* Use the current slot if we can do so without exceeding the load factor. */
    if (t->used + 1 < LOAD_FACTOR * t->capacity)
    {
	*out = (struct phbase_ptrdex_locator){
	    .locator = PRESENT_BIT | hash,
	    .slot = ix,
	};

	return PHBASE_STATUS_OK;
    }

    /* If we reach here, then we need to grow the table to accomodate the new
     * value. After a basic check, create a new empty table, and then repack
     * the old values into the new table. */

    /* Check growing capacity doesn't overflow. */
    if (2 * t->capacity < t->capacity)
    {
	return PHBASE_STATUS_NO_MEMORY;
    }

    struct phbase_ptrdex__internal new = { 0 };
    int rc = phbase_ptrdex__init_with_capacity(&new, 2 * t->capacity, value_size);

    if (rc != PHBASE_STATUS_OK)
    {
	/* Remap too much capacity to fullness. */
	if (rc == PHBASE_STATUS_INVALID_PARAMETER)
	{
	    return PHBASE_STATUS_FULL;
	}
	else
	{
	    return rc;
	}
    }

    repack(&new, t, value_size);
    rc = phbase_ptrdex__obtain_slot(&new, key, out, value_size);
    PHBASE_ASSERT(rc == PHBASE_STATUS_OK);

    /* Replace the old table with the new one. */
    phbase_ptrdex__clear(t);
    *t = new;

    return PHBASE_STATUS_OK;
}
