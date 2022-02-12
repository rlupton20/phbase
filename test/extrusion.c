#include <phbase/extrusion.h>

#include <phbase/core.h>
#include <phbase/test.h>

static void
test_extrusion(phbase_test_ctx* ctx)
{
    struct full {
	int x;
	int y;
    } v = { 0 };

    const struct full* recovered = PHBASE_EXTRUDE(&v.y, struct full, y);

    if (recovered != &v)
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_EXTRUDE(&v.y, struct full, y) = %p, but %p was expected",
			  (void*)recovered, (void*)&v);
    }
}

static void
test_extrusion_base(phbase_test_ctx* ctx)
{
    struct full {
	int x;
	int y;
    } v = { 0 };

    const struct full* recovered = PHBASE_EXTRUDE(&v.x, struct full, x);

    if (recovered != &v)
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_EXTRUDE(&v.x, struct full, x) = %p, but %p was expected",
			  (void*)recovered, (void*)&v);
    }
}

static void
test_extrusion_with_anonymous_union(phbase_test_ctx* ctx)
{
    struct full {
	int x;
	union {
	    int y;
	    long int z;
	};
    } v = { 0 };

    const struct full* recovered = PHBASE_EXTRUDE(&v.y, struct full, y);

    if (recovered != &v)
    {
	PHBASE_TEST_ERROR(ctx, "PHBASE_EXTRUDE(&v.y, struct full, y) = %p, but %p was expected",
			  (void*)recovered, (void*)&v);
    }
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_extrusion),
    PHBASE_TEST_RECORD(test_extrusion_base),
    PHBASE_TEST_RECORD(test_extrusion_with_anonymous_union),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/extrusion",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_extrusion_suite = &suite;
