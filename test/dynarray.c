#include <phbase/dynarray.h>

#include <phbase/core.h>
#include <phbase/test.h>

#include <assert.h>
#include <stdalign.h>

static void
test_dynarray_support(phbase_test_ctx* ctx)
{
    if (!PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(int))
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(int) = false, where true was expected");
    }

    struct maximal {
	alignas(alignof(max_align_t)) int x;
    };

    if (!PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(struct maximal))
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(struct maximal) = false, where true was expected");
    }

    struct excessive {
	alignas(2 * alignof(max_align_t)) int x;
    };

    if (PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(struct excessive))
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(struct excessive) = true, where false was expected");
    }
}

static void
test_dynarray_init(phbase_test_ctx* ctx)
{
    static const size_t TEST_CAPACITY = 128;

    PHBASE_DYNARRAY size_t* a = { 0 };

    if (phbase_dynarray_init_with_capacity(&a, TEST_CAPACITY) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize dynamic array a");
	goto fatal;
    }

    if (!a)
    {
	PHBASE_TEST_ERROR(ctx, "a was initialized but is NULL");
	goto fatal;
    }

    size_t length = phbase_dynarray_length(&a);

    if (length != 0)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_length(a) = %zu, where %zu was expected",
			  length, 0);
    }

    size_t capacity = phbase_dynarray_capacity(&a);

    if (capacity != TEST_CAPACITY)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_capacity(a) = %zu, where %zu was expected",
			  capacity, TEST_CAPACITY);
    }

    for (size_t ix = 0; ix < TEST_CAPACITY; ++ix)
    {
	a[ix] = ix;
    }

    for (size_t ix = 0; ix < TEST_CAPACITY; ++ix)
    {
	if(a[ix] != ix)
	{
	    PHBASE_TEST_ERROR(ctx, "a[%zu] == %d, but %d was expected\n", ix, a[ix], ix);
	}
    }

fatal:
    if (a)
    {
	phbase_dynarray_clear(&a);
    }

    return;
}

static void
test_dynarray_push(phbase_test_ctx* ctx)
{
    static const size_t TEST_CAPACITY = 8;

    /* Often dynamic arrays are part of bigger structures - check
       declaring them as such still works. */
    struct {
	PHBASE_DYNARRAY size_t* a;
    } s = { 0 };

    if (phbase_dynarray_init_with_capacity(&s.a, TEST_CAPACITY))
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize dynamic array s.a");
	goto fatal;
    }

    if (!s.a)
    {
	PHBASE_TEST_ERROR(ctx, "s.a was intialized but is NULL");
	goto fatal;
    }

    for (size_t ix = 0; ix < TEST_CAPACITY; ++ix)
    {
	if (phbase_dynarray_push(&s.a, ix))
	{
	    PHBASE_TEST_ERROR(ctx, "failed %zu'th push, pushing %zu to s.a", ix, ix);
	    goto fatal;
	}
    }

    if (phbase_dynarray_length(&s.a) != TEST_CAPACITY)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_length(&s.a) = %zu, but %zu was expected",
			  phbase_dynarray_length(&s.a), TEST_CAPACITY);
	goto fatal;
    }

    if (phbase_dynarray_capacity(&s.a) != TEST_CAPACITY)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_capacity(&s.a) = %zu, but %zu was expected",
			  phbase_dynarray_length(&s.a), TEST_CAPACITY);
	goto fatal;
    }

    /* Now we check that we can grow the array beyond capacity */
    if (phbase_dynarray_push(&s.a, TEST_CAPACITY))
    {
	PHBASE_TEST_ERROR(ctx, "failed to dynamically grow array");
	goto fatal;
    }

    if (phbase_dynarray_length(&s.a) != TEST_CAPACITY + 1)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_length(&s.a) = %zu, but %zu was expected",
			  phbase_dynarray_length(&s.a), TEST_CAPACITY + 1);
    }

    if (phbase_dynarray_capacity(&s.a) <= TEST_CAPACITY)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_capacity(&s.a) = %zu, but expected at least %zu",
			  phbase_dynarray_length(&s.a), TEST_CAPACITY + 1);
	goto fatal;
    }

    /* Check that all the pushed data is as expected*/
    for (size_t ix = 0; ix < TEST_CAPACITY + 1; ++ix)
    {
	if (s.a[ix] != ix)
	{
	    PHBASE_TEST_ERROR(ctx, "s.a[%zu] == %d, but %d was expected\n", ix, s.a[ix], ix);
	}
    }

fatal:
    if (s.a)
    {
	phbase_dynarray_clear(&s.a);
    }
    return;
}

static void
test_dynarray_pop(phbase_test_ctx* ctx)
{
    PHBASE_DYNARRAY int* a = { 0 };

    if (phbase_dynarray_init_with_capacity(&a, 8) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize dynamic array a");
	goto fatal;
    }

    if (!a)
    {
	PHBASE_TEST_ERROR(ctx, "a was intialized but is NULL");
	goto fatal;
    }

    if (phbase_dynarray_push(&a, 42))
    {
	PHBASE_TEST_ERROR(ctx, "failed to push value to initialized array");
	goto fatal;
    }

    int out = 0;

    if (phbase_dynarray_pop(&a, &out))
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_pop(&a, &out) failed when a was non-empty");
	goto fatal;
    }

    if (out != 42)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_pop(&a, &out) wrote %d to out, but %d was expected",
			  out, 42);
    }

    if (!phbase_dynarray_pop(&a, &out))
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_pop(&a, &out) succeeded when a was empty");
	goto fatal;
    }

    if (out != 42)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_pop(&a, &out) failed but overwrote out - was %d, now %d",
			  42, out);
    }

fatal:
    if (a)
    {
	phbase_dynarray_clear(&a);
    }
    return;
}

static inline size_t
supremum_capacity(size_t size)
{
    assert(size != 0);
    static const size_t high_bit = 1UL << (sizeof(size_t) * 8 - 1); /* (SIZE_MAX + 1) / 2 */
    const size_t capacity = 2 * (high_bit / size);
    assert(capacity != 0 && capacity * size == 0);
    return capacity;
}

static void
test_dynarray_overflow(phbase_test_ctx* ctx)
{
    PHBASE_DYNARRAY int* a = { 0 };

    size_t capacity = supremum_capacity(sizeof(int));

    if (phbase_dynarray_init_with_capacity(&a, capacity) != PHBASE_STATUS_NO_MEMORY)
    {
	PHBASE_TEST_ERROR(ctx, "initialized array despite overflow of size_t");
	goto fatal;
    }

    if (a)
    {
	PHBASE_TEST_ERROR(ctx, "a has a non-zero value despite failed initialization");
    }

fatal:
    if (a)
    {
	phbase_dynarray_clear(&a);
    }

    return;
}

static void
test_dynarray_array_types(phbase_test_ctx* ctx)
{
    float (PHBASE_DYNARRAY *vecs)[3] = { 0 };
    static_assert(sizeof(vecs[0]) == 3 * sizeof(float), "expected an array of float[3]");

    if (phbase_dynarray_init_with_capacity(&vecs, 8))
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize vecs");
	goto fatal;
    }

    if (!vecs)
    {
	PHBASE_TEST_ERROR(ctx, "vecs is NULL despite successful initialization");
    }

    vecs[0][2] = 3.141;

fatal:
    if (vecs)
    {
	phbase_dynarray_clear(&vecs);
    }

    return;
}

static void
test_dynarray_proc_types(phbase_test_ctx* ctx)
{
    void (PHBASE_DYNARRAY **procs)(phbase_test_ctx* ctx) = { 0 };

    if (phbase_dynarray_init_with_capacity(&procs, 8))
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize vecs");
	goto fatal;
    }

    if (!procs)
    {
	PHBASE_TEST_ERROR(ctx, "procs is NULL despite successful initialization");
    }

    procs[0] = test_dynarray_proc_types;

fatal:
    if (procs)
    {
	phbase_dynarray_clear(&procs);
    }

    return;
}

static void
test_dynarray_set_length(phbase_test_ctx* ctx)
{
    PHBASE_DYNARRAY int* da = { 0 };

    if (phbase_dynarray_init_with_capacity(&da, 8))
    {
	PHBASE_TEST_ERROR(ctx, "failed to initialize da");
	goto fatal;
    }

    size_t length = phbase_dynarray_length(&da);

    if (length != 0)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_length(&da) = %zu, but %zu was expected",
			  length, 0);
	goto fatal;
    }

    if (phbase_dynarray_set_length(&da, 8) != PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "failed to set length of da to capacity");
	goto fatal;
    }

    if (phbase_dynarray_set_length(&da, 9) == PHBASE_STATUS_OK)
    {
	PHBASE_TEST_ERROR(ctx, "phbase_dynarray_set_length allowed length to be set beyond capacity");
	goto fatal;
    }

fatal:
    if (da)
    {
	phbase_dynarray_clear(&da);
    }

    return;
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_dynarray_support),
    PHBASE_TEST_RECORD(test_dynarray_init),
    PHBASE_TEST_RECORD(test_dynarray_push),
    PHBASE_TEST_RECORD(test_dynarray_pop),
    PHBASE_TEST_RECORD(test_dynarray_overflow),
    PHBASE_TEST_RECORD(test_dynarray_array_types),
    PHBASE_TEST_RECORD(test_dynarray_proc_types),
    PHBASE_TEST_RECORD(test_dynarray_set_length),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/dynarray",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_dynarray_suite = &suite;
