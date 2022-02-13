#include <phbase/core.h>
#include <phbase/test.h>

#include <stdlib.h>

extern const struct phbase_test_testsuite* phbase_core_suite;
extern const struct phbase_test_testsuite* phbase_utf8_suite;
extern const struct phbase_test_testsuite* phbase_extrusion_suite;
extern const struct phbase_test_testsuite* phbase_dynarray_suite;
extern const struct phbase_test_testsuite* phbase_allocator_arena_suite;

int
main(int argc, char* argv[])
{
    PHBASE_UNUSED_PARAMETER(argc);
    PHBASE_UNUSED_PARAMETER(argv);

    struct phbase_test_results results = { 0 };

    phbase_test_run_testsuite(phbase_core_suite, &results);
    phbase_test_run_testsuite(phbase_utf8_suite, &results);
    phbase_test_run_testsuite(phbase_extrusion_suite, &results);
    phbase_test_run_testsuite(phbase_dynarray_suite, &results);

    phbase_test_run_testsuite(phbase_allocator_arena_suite, &results);

    return phbase_test_summarize_results(&results);
}
