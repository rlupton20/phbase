#include <phbase/ptrdex.h>

#include <phbase/core.h>
#include <phbase/status.h>
#include <phbase/test.h>

static void
test_ptrdex_init_and_clear(phbase_test_ctx* ctx)
{
    union {
	struct phbase_ptrdex__internal internal;
	struct {
	    uint32_t* key;
	    uint32_t* value;
	};
    } ht = { 0 };

    if (phbase_ptrdex_is_initialized(&ht))
    {
	PHBASE_TEST_ERROR(ctx, "zeroed ptrdex-table registers as initialized");
    }

    if (phbase_ptrdex_init_with_capacity(&ht, 8) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize ptrdex table\n");
	goto fatal;
    }

    phbase_ptrdex_clear(&ht);

    if (phbase_ptrdex_is_initialized(&ht))
    {
	PHBASE_TEST_ERROR(ctx, "cleared ptrdex-table registers as initialized");
    }

fatal:
    return;
}

static void
test_ptrdex_obtain_slot(phbase_test_ctx* ctx)
{
    union {
	struct phbase_ptrdex__internal internal;
	struct {
	    uint32_t** key;
	    uint32_t* value;
	};
    } ht = { 0 };

    uint32_t keys[] = { 0, 1, 2 };

    if (phbase_ptrdex_init_with_capacity(&ht, 8) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize ptrdex table\n");
	goto fatal;
    }

    /* Insert each key-value pair. */
    for (uint32_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(keys); ++ix)
    {
	struct phbase_ptrdex_locator l = { 0 };

	if (phbase_ptrdex_obtain_slot(&ht, keys + ix, &l) != PHBASE_STATUS_OK)
	{
	    PHBASE_TEST_ERROR(ctx, "error obtaining ptrdex slot for key %zu", ix);
	    goto fatal;
	}

	phbase_ptrdex_insert(&ht, l, keys + ix, keys[ix]);
    }

    /* Check we can find each key-value pair.. */
    for (uint32_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(keys); ++ix)
    {
	size_t slot = 0;

	if (phbase_ptrdex_find(&ht, keys + ix, &slot) != PHBASE_STATUS_OK)
	{
	    PHBASE_TEST_ERROR(ctx, "error finding key number %u", ix);
	    continue;
	}

	if (ht.key[slot] != keys + ix)
	{
	    PHBASE_TEST_ERROR(ctx, "key check %zu failed: expected %p, found %p",
			      ix, keys + ix, ht.key[slot]);
	}

	if (ht.value[slot] != keys[ix])
	{
	    PHBASE_TEST_ERROR(ctx, "value check %zu failed: expected %d, found %d",
			      ix, keys[ix], ht.value[slot]);
	}
    }

fatal:
    if (phbase_ptrdex_is_initialized(&ht))
    {
	phbase_ptrdex_clear(&ht);
    }

    return;
}

static void
test_ptrdex_obtain_slot_grow(phbase_test_ctx* ctx)
{
    union {
	struct phbase_ptrdex__internal internal;
	struct {
	    uint32_t** key;
	    uint32_t* value;
	};
    } ht = { 0 };

    uint32_t keys[32] = { 0 };

    for (uint32_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(keys); ++ix)
    {
	keys[ix] = ix;
    }

    if (phbase_ptrdex_init_with_capacity(&ht, 8) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize ptrdex table\n");
	goto fatal;
    }

    size_t notgrown = ht.internal.capacity;

    /* Insert each key-value pair. */
    for (uint32_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(keys); ++ix)
    {
	struct phbase_ptrdex_locator l = { 0 };

	if (phbase_ptrdex_obtain_slot(&ht, keys + ix, &l) != PHBASE_STATUS_OK)
	{
	    PHBASE_TEST_ERROR(ctx, "error obtaining ptrdex slot for key %u", ix);
	    goto fatal;
	}

	phbase_ptrdex_insert(&ht, l, keys + ix, keys[ix]);
	notgrown &= ht.internal.capacity;
    }

    /* Check the hash-table had to increase in size. */
    if (notgrown)
    {
	PHBASE_TEST_ERROR(ctx, "ptrdex did not grow across insertions");
	goto fatal;
    }

    /* Check we can find each key-value pair. */
    for (uint32_t ix = 0; ix < PHBASE_STATIC_ARRAY_LENGTH(keys); ++ix)
    {
	size_t slot = 0;

	if (phbase_ptrdex_find(&ht, keys + ix, &slot) != PHBASE_STATUS_OK)
	{
	    PHBASE_TEST_ERROR(ctx, "error finding key number %u", ix);
	    continue;
	}

	if (ht.key[slot] != keys + ix)
	{
	    PHBASE_TEST_ERROR(ctx, "key check %zu failed: expected %p, found %p",
			      ix, keys + ix, ht.key[slot]);
	}

	if (ht.value[slot] != keys[ix])
	{
	    PHBASE_TEST_ERROR(ctx, "value check %zu failed: expected %d, found %d",
			      ix, keys[ix], ht.value[slot]);
	}
    }

fatal:
    if (phbase_ptrdex_is_initialized(&ht))
    {
	phbase_ptrdex_clear(&ht);
    }

    return;
}

static void
test_ptrdex_overflow(phbase_test_ctx* ctx)
{
    struct value {
	/* Some big arrays */
	uint8_t a[1UL << (sizeof(size_t) * 8 - 32)];
	uint8_t b[1UL << (sizeof(size_t) * 8 - 32)];
    };

    union {
	struct phbase_ptrdex__internal internal;
	struct {
	    uint32_t** key;
	    struct value* value;
	};
    } ht = { 0 };

    int rc = phbase_ptrdex_init_with_capacity(&ht, ~(1U << 31));

    if (rc == PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "allocated ptrdex with unallocatable memory size "
			       "(would cause size_t overflow)");
    }
    else if (rc != PHBASE_STATUS_NO_MEMORY)
    {
	PHBASE_TEST_ERROR(ctx, "allocating ptrdex failed, but for an unexpected reason");
    }
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_ptrdex_init_and_clear),
    PHBASE_TEST_RECORD(test_ptrdex_obtain_slot),
    PHBASE_TEST_RECORD(test_ptrdex_obtain_slot_grow),
    PHBASE_TEST_RECORD(test_ptrdex_overflow),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/ptrdex",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_ptrdex_suite = &suite;
