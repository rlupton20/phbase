#include <phbase/allocator.h>

/*
 * libc allocator
 */
#include <phbase/core.h>
#include <stdlib.h>

static void*
libc_realloc(const struct phbase_allocator* a, void* ptr, size_t size)
{
    PHBASE_UNUSED_PARAMETER(a);
    return realloc(ptr, size);
}

const struct phbase_allocator phbase_allocator_libc = {
    .realloc = libc_realloc,
};
