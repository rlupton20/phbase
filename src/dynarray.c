#include <phbase/dynarray.h>

#include <phbase/extrusion.h>

#include "allocation.h"

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h> /* realloc, free */
#include <string.h> /* memcpy */

#define PHBASE_ASSERT assert
#define PHBASE_MEMCPY memcpy

struct dynarray
{
    size_t capacity;
    size_t used;
    alignas(alignof(max_align_t)) uint8_t data[];
};

static inline struct dynarray*
dynarray_realloc(void* ptr, size_t count, size_t size)
{
    const size_t total_size = sizeof(struct dynarray) + size * count;

    /* Check for arithmetic overflow. */
    if (array_allocates(count, size) && total_size > size * count)
    {
	return realloc(ptr, total_size);
    }
    else
    {
	return NULL;
    }
}

int
phbase_dynarray__init_with_capacity(void** slots, size_t count, size_t size)
{
    if (!count)
    {
	count = 1;
    }

    struct dynarray* a = dynarray_realloc(NULL, count, size);

    if (!a)
    {
	return PHBASE_STATUS_NO_MEMORY;
    }

    a->used = 0;
    a->capacity = count;
    *slots = &(a->data);

    return PHBASE_STATUS_OK;
}

void
phbase_dynarray__clear(void** slots)
{
    struct dynarray* a = PHBASE_EXTRUDE(*slots, struct dynarray, data);
    free(a);
    *slots = NULL;
}

size_t
phbase_dynarray__length(void** slots)
{
    return PHBASE_EXTRUDE(*slots, struct dynarray, data)->used;
}

size_t
phbase_dynarray__capacity(void** slots)
{
    return PHBASE_EXTRUDE(*slots, struct dynarray, data)->capacity;
}

int
phbase_dynarray__ensure_capacity(void** slots, size_t count, size_t size)
{
    struct dynarray* a = PHBASE_EXTRUDE(*slots, struct dynarray, data);

    if (a->used + count <= a->capacity)
    {
	return PHBASE_STATUS_OK;
    }

    /* We need to grow the array. */
    size_t new_capacity = a->capacity;

    while (new_capacity < a->used + count)
    {
	/* Check we don't overflow new_capacity. */
	if (new_capacity * 2 <= new_capacity)
	{
	    return PHBASE_STATUS_NO_MEMORY;
	}

	new_capacity = new_capacity * 2;
    }

    a = dynarray_realloc(a, new_capacity, size);

    if (!a)
    {
	return PHBASE_STATUS_NO_MEMORY;
    }

    a->capacity = new_capacity;
    *slots = &(a->data);
    return PHBASE_STATUS_OK;
}

int
phbase_dynarray__set_length(void** slots, size_t length)
{
    struct dynarray* a = PHBASE_EXTRUDE(*slots, struct dynarray, data);

    if (length > a->capacity)
    {
	return PHBASE_STATUS_INVALID_PARAMETER;
    }

    a->used = length;
    return PHBASE_STATUS_OK;
}

size_t
phbase_dynarray__claim_back(void** slots)
{
    struct dynarray* a = PHBASE_EXTRUDE(*slots, struct dynarray, data);
    PHBASE_ASSERT(a->used < a->capacity);
    return a->used++;
}

int
phbase_dynarray__pop(void** slots, void* out, size_t size)
{
    struct dynarray* a = PHBASE_EXTRUDE(*slots, struct dynarray, data);

    if (a->used == 0)
    {
	return PHBASE_STATUS_DOES_NOT_EXIST;
    }

    const char* const base = ((char*)*slots) + (size * --a->used);
    PHBASE_MEMCPY(out, base, size);

    return PHBASE_STATUS_OK;
}
