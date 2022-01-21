#ifndef PHBASE_TEST_H
#define PHBASE_TEST_H

#include <stddef.h>
#include <stdint.h>

#define PHBASE_TEST_RECORD(t) { .name = #t, .test = (t) }
#define PHBASE_TEST_ERROR(ctx, ...) phbase__test_ctx_error(ctx, __VA_ARGS__);

typedef struct phbase_test_ctx phbase_test_ctx;

struct phbase_test
{
    const char* const name;
    void (*test)(phbase_test_ctx* ctx);
};

struct phbase_test_testsuite
{
    const char* name;
    size_t length;
    const struct phbase_test* tests;
};

struct phbase_test_results
{
    uintmax_t passed;
    uintmax_t failed;
};

void
phbase_test_run_testsuite(const struct phbase_test_testsuite* suite,
			  struct phbase_test_results* results);

int
phbase_test_summarize_results(const struct phbase_test_results* results);

void
phbase__test_ctx_error(struct phbase_test_ctx* ctx, const char* fmt, ...);

#endif
