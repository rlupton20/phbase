#include <phbase/bitarray.h>

#include <phbase/core.h>
#include <phbase/test.h>

#include <stdlib.h>

_Static_assert(PHBASE_BITARRAY_MEMORY_ALIGN <= alignof(max_align_t),
	       "phbase_bitarray memory should be aligned with standard allocator");

static void
test_bitarray_operations(struct phbase_test_ctx* ctx)
{
    void* memory = calloc(1, PHBASE_BITARRAY_MEMORY_SIZE(32));

    if (!memory)
    {
	PHBASE_TEST_ERROR(ctx, "failed to allocate memory for bitarray");
	goto fatal;
    }

    struct phbase_bitarray ba = phbase_bitarray_from_memory(memory);

    phbase_bitarray_set_bit(&ba, 3);
    phbase_bitarray_set_bit(&ba, 4);
    phbase_bitarray_set_bit(&ba, 9);

    if (phbase_bitarray_bit_is_set(&ba, 2))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 2 was not set, but reports that it is so");
    }

    if (!phbase_bitarray_bit_is_set(&ba, 3))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 3 was set, but doesn't return that it is so");
    }

    if (!phbase_bitarray_bit_is_set(&ba, 4))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 4 was set, but doesn't return that it is so");
    }

    if (!phbase_bitarray_bit_is_set(&ba, 9))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 9 was set, but doesn't return that it is so");
    }

    phbase_bitarray_set_bit(&ba, 2);
    phbase_bitarray_clear_bit(&ba, 3);
    phbase_bitarray_clear_bit(&ba, 9);

    if (!phbase_bitarray_bit_is_set(&ba, 2))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 2 was set, but doesn't return that it is so");
    }

    if (phbase_bitarray_bit_is_set(&ba, 3))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 3 was not set, but reports that it is so");
    }

    if (!phbase_bitarray_bit_is_set(&ba, 4))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 4 was set, but doesn't return that it is so");
    }

    if (phbase_bitarray_bit_is_set(&ba, 9))
    {
	PHBASE_TEST_ERROR(ctx, "bitarray bit 9 was not set, but reports that it is so");
    }

fatal:
    if (memory)
    {
	free(memory);
    }

    return;
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_bitarray_operations),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/bitarray",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_bitarray_suite = &suite;
