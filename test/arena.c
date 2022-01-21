#include <phbase/arena.h>

#include <phbase/core.h>
#include <phbase/test.h>

#include <assert.h>
#include <stdalign.h>

#define PHBASE_ASSERT assert

static void
test_arena_basic(phbase_test_ctx* ctx)
{
    char buffer[128] = {0};

    struct phbase_arena a = {0};

    phbase_arena_init(&a, 128, buffer);

    for (uint32_t ix = 0; ix < 128; ++ix)
    {
	if (!phbase_arena_allocate(&a, alignof(char), sizeof(char)))
	{
	    PHBASE_TEST_ERROR(ctx, "Failed to allocate byte %u/%u\n", ix, 128 - 1);
	    goto error;
	}
    }

    if (phbase_arena_allocate(&a, alignof(char), sizeof(char)))
    {
	PHBASE_TEST_ERROR(ctx, "Allocated beyond end of memory buffer\n");
	goto error;
    }

    phbase_arena_reset(&a);

    for (uint32_t ix = 0; ix < 128; ++ix)
    {
	if (!phbase_arena_allocate(&a, alignof(char), sizeof(char)))
	{
	    PHBASE_TEST_ERROR(ctx, "Failed to allocate byte %u/%u after reset\n", ix, 128 - 1);
	    goto error;
	}
    }

    if (phbase_arena_allocate(&a, alignof(char), sizeof(char)))
    {
	PHBASE_TEST_ERROR(ctx, "Allocated beyond end of memory buffer after reset\n");
	goto error;
    }

error:
    return;
}

static void
test_arena_alignment(phbase_test_ctx* ctx)
{
    char buffer[128] = {0};

    struct phbase_arena a = {0};

    phbase_arena_init(&a, 128, buffer);

    struct {
	size_t align;
	size_t size;
    } allocations[] = {
	{ .align = alignof(uint8_t),  .size = sizeof(uint8_t)  },
	{ .align = alignof(uint16_t), .size = sizeof(uint16_t) },
	{ .align = alignof(uint32_t), .size = sizeof(uint32_t) },
	{ .align = alignof(uint8_t),  .size = sizeof(uint8_t)  },
	{ .align = alignof(uint64_t), .size = sizeof(uint64_t) },
    };

    for (size_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(allocations); ++ix)
    {
	void* p = phbase_arena_allocate(&a, allocations[ix].align, allocations[ix].size);

	if (!p)
	{
	    PHBASE_TEST_ERROR(ctx, "Allocation %zu (align: %zu, size: %zu) failed\n", ix,
			      allocations[ix].align, allocations[ix].size);
	}

	size_t bits = (uintptr_t)p;
	size_t zeros = bits & (allocations[ix].align - 1);

	if (zeros)
	{
	    PHBASE_TEST_ERROR(ctx, "Allocation %zu (align: %zu, size: %zu) is not aligned: masked %zu\n",
			      ix, allocations[ix].align, allocations[ix].size, zeros);
	}
    }
}

static void
test_arena_size_wrap(phbase_test_ctx* ctx)
{
    char buffer[128] = {0};

    struct phbase_arena a = {0};

    phbase_arena_init(&a, 128, buffer);

    if (phbase_arena_allocate(&a, alignof(char), SIZE_MAX))
    {
	PHBASE_TEST_ERROR(ctx, "Allocated a buffer of size SIZE_MAX");
    }
}

static void
test_arena_align_wrap(phbase_test_ctx* ctx)
{
    struct phbase_arena a = {0};

    phbase_arena_init(&a, SIZE_MAX - 1, (char*)1);

    if (!phbase_arena_allocate(&a, alignof(char), SIZE_MAX - 2))
    {
	PHBASE_TEST_ERROR(ctx, "Allocated a buffer of size SIZE_MAX");
    }

    PHBASE_ASSERT((a.current & 3) != 0);
    PHBASE_ASSERT(a.current + 3 < a.current);

    if (phbase_arena_allocate(&a, alignof(uint32_t), sizeof(uint32_t)))
    {
	PHBASE_TEST_ERROR(ctx, "Allocated uint32_t despite align overflow");
    }
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_arena_basic),
    PHBASE_TEST_RECORD(test_arena_alignment),
    PHBASE_TEST_RECORD(test_arena_size_wrap),
    PHBASE_TEST_RECORD(test_arena_align_wrap),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/allocator/arena",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_allocator_arena_suite = &suite;
