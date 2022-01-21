#include <phbase/test.h>

#include <phbase/termcodes.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define TTY_PASS ANSI_GREEN ANSI_BOLD "PASS" ANSI_RESET
#define TTY_FAIL ANSI_RED   ANSI_BOLD "FAIL" ANSI_RESET

struct phbase_test_ctx
{
    bool error;
};

static int
writev(const char* fmt, va_list ap)
{
    return vfprintf(stdout, fmt, ap);
}

static int
write(const char* fmt, ...)
{
    va_list ap  = {0};
    va_start(ap, fmt);
    int ret = writev(fmt, ap);
    va_end(ap);
    return ret;
}

static const struct {
    int (*log)(const char* fmt, ...);
    int (*logv)(const char* fmt, va_list ap);
} out = {
    .log = write,
    .logv = writev
};

void
phbase__test_ctx_error(struct phbase_test_ctx* ctx, const char* fmt, ...)
{
    ctx->error = true;

    va_list ap  = {0};
    va_start(ap, fmt);
    out.log("\n");
    out.logv(fmt, ap);
    out.log("\n");
    va_end(ap);
}

void
phbase_test_run_testsuite(const struct phbase_test_testsuite* suite,
			  struct phbase_test_results* results)
{
    out.log(ANSI_BOLD "%s\n" ANSI_RESET, suite->name);

    for (size_t ix = 0; ix < suite->length; ++ix)
    {
        phbase_test_ctx ctx = { 0 };
        out.log("Running %s...", suite->tests[ix].name);
        suite->tests[ix].test(&ctx);

        if (ctx.error)
        {
            results->failed++;
            out.log("\r%-50s" TTY_FAIL "\n", suite->tests[ix].name);
        }
        else
        {
            out.log("\r%-50s" TTY_PASS "\n", suite->tests[ix].name);
            results->passed++;
        }
    }

    out.log("\n");
}

int
phbase_test_summarize_results(const struct phbase_test_results* results)
{
    out.log("\n\nPASS: " ANSI_BOLD "%s%ld" ANSI_RESET "\n",
            results->failed > 0 ? "" : ANSI_GREEN, results->passed);
    out.log("FAIL: " ANSI_BOLD "%s%ld" ANSI_RESET "\n",
	    results->failed > 0 ? ANSI_RED : "", results->failed);

    return results->failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
