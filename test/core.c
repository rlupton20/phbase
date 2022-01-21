#include <phbase/core.h>

#include <phbase/test.h>

static void
test_static_array_length(phbase_test_ctx* ctx)
{
    int array[] = { 1,2,3,4 };

    size_t got = PHBASE_STATIC_ARRAY_LENGTH(array);

    if (got != 4)
    {
	PHBASE_TEST_ERROR(ctx, "PHSTD_STATIC_ARRAY_LENGTH(array) = %zu, but %zu was expected",
			  got, 4);
    }
}

static void
test_static_strlen(phbase_test_ctx* ctx)
{
    size_t got = PHBASE_STATIC_STRLEN("static string");

    if (got != 13)
    {
	PHBASE_TEST_ERROR(ctx, "PHSTD_STATIC_STRLEN(\"static string\") = %zu, but %zu was expected",
			  got, 13);
    }
}

static void
test_static_strlen_empty_string(phbase_test_ctx* ctx)
{
    size_t got = PHBASE_STATIC_STRLEN("");

    if (got != 0)
    {
	PHBASE_TEST_ERROR(ctx, "PHSTD_STATIC_STRLEN(\"\") = %zu, but %zu was expected",
			  got, 0);
    }
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_static_array_length),
    PHBASE_TEST_RECORD(test_static_strlen),
    PHBASE_TEST_RECORD(test_static_strlen_empty_string),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/core",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_core_suite = &suite;
