#include <phbase/utf8.h>

#include <phbase/core.h>
#include <phbase/test.h>

#include <string.h>

static void
test_decode_ascii(phbase_test_ctx* ctx)
{
    const char* const string = "a";
    phbase_codepoint c = 0;

    size_t bytes = phbase_utf8_decode_character(&c, strlen(string), (uint8_t*)string);
    if (bytes == 0)
    {
        PHBASE_TEST_ERROR(ctx, "Failed to decode starting byte for '%s'", string);
        goto fatal;
    }

    if (bytes != 1)
    {
        PHBASE_TEST_ERROR(ctx,
			  "Expected to decode a single byte for ASCII character, but "
			  "consumed %ld bytes instead",
			  bytes);
        goto fatal;
    }

    if (c != 'a')
    {
        PHBASE_TEST_ERROR(ctx, "Expected to decode ASCII character 'a' but got '%lc' instead", c);
    }

fatal:
    return;
}

static void
test_decode_empty(phbase_test_ctx* ctx)
{
    size_t bytes = phbase_utf8_decode_character(&(phbase_codepoint){ 0 }, 0, (uint8_t[1]){ 0 });

    if (bytes > 0)
    {
        PHBASE_TEST_ERROR(ctx, "Decode of empty bytestring decoded %zu bytes where 0 was expected",
			  bytes);
    }
}

static void
test_decode_lambda_character(phbase_test_ctx* ctx)
{
    const uint8_t string[] = { 0xCE, 0xBB };
    const phbase_codepoint lambda = 0x03BB;
    phbase_codepoint c = 0;

    size_t bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(string), string);

    if (bytes == 0)
    {
        // @TODO :UTF-8: Correct printing here relies on having a UTF-8 locale
        PHBASE_TEST_ERROR(ctx, "Failed to decode '%s'", string);
        goto fatal;
    }

    if (bytes != 2)
    {
        PHBASE_TEST_ERROR(ctx,
			  "Expected to decode a single byte for ASCII character, but "
			  "consumed %ld byte%s instead",
			  bytes, bytes == 1 ? "" : "s");
        goto fatal;
    }

    if (c != lambda)
    {
        PHBASE_TEST_ERROR(ctx, "Expected to decode ASCII character '%lc' but got '%lc' instead",
			  lambda, c);
    }

fatal:
    return;
}

static void
test_decode_bad_continuation(phbase_test_ctx* ctx)
{
    const uint8_t string[] = { 0xCE, 0xC0 };
    phbase_codepoint c = 0;

    size_t bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(string), string);

    if (bytes != 0)
    {
        PHBASE_TEST_ERROR(ctx, "Expected decode failure but decoded %ld byte%s to obtain '%lc'",
			  bytes, bytes == 1 ? "" : "s", c);
        goto fatal;
    }

fatal:
    return;
}

static void
test_decode_bad_initial_byte(phbase_test_ctx* ctx)
{
    const uint8_t string[] = {
        0xFF,
    };
    phbase_codepoint c = 0;

    size_t bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(string), string);

    if (bytes != 0)
    {
        PHBASE_TEST_ERROR(ctx, "Expected decode failure but decoded %ld byte%s to obtain '%lc'",
                     bytes, bytes == 1 ? "" : "s", c);
        goto fatal;
    }

fatal:
    return;
}

static void
test_decode_long_encoded_ascii(phbase_test_ctx* ctx)
{
    const uint8_t string[] = { 0xC0, (0x80 | '!') };
    phbase_codepoint c = 0;

    size_t bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(string), string);

    if (bytes != 0)
    {
        PHBASE_TEST_ERROR(ctx, "Expected decode failure but decoded %ld byte%s to obtain '%lc'",
                     bytes, bytes == 1 ? "" : "s", c);
        goto fatal;
    }

fatal:
    return;
}

static void
test_character_length_boundaries(phbase_test_ctx* ctx)
{
    struct
    {
        char* description;
        uint8_t bytes[6];
        struct
        {
            size_t num_bytes;
            phbase_codepoint codepoint;
        } expected;
    } tests[] = {
	{
	    .description = "Decode lambda (testing the test)",
	    .bytes = { 0xCE, 0xBB },
	    .expected = {
		.num_bytes = 2,
		.codepoint = 0x03BB,
	    }
	},
	{
	    .description = "Decode last ASCII character",
	    .bytes = { 0x70 },
	    .expected = {
		.num_bytes = 1,
		.codepoint = 0x70,
	    }
	},
	{
	    .description = "Decode first 2-byte character",
	    .bytes = { 0xC2, 0x80 },
	    .expected = {
		.num_bytes = 2,
		.codepoint = 0x80,
	    }
	},
	{
	    .description = "Decode last 2-byte character",
	    .bytes = { 0xDF, 0xBF },
	    .expected = {
		.num_bytes = 2,
		.codepoint = 0x07FF,
	    }
	},
	{
	    .description = "Decode first 3-byte character",
	    .bytes = { 0xE0, 0xA0, 0x80 },
	    .expected = {
		.num_bytes = 3,
		.codepoint = 0x0800,
	    }
	},
	{
	    .description = "Decode last 3-byte character",
	    .bytes = { 0xEF, 0xBF, 0xBF },
	    .expected = {
		.num_bytes = 3,
		.codepoint = 0xFFFF,
	    },
	},
	{
	    .description = "Decode first 4-byte character",
	    .bytes = { 0xF0, 0x90, 0x80, 0x80 },
	    .expected = {
		.num_bytes = 4,
		.codepoint = 0x10000,
	    },
	},
	{
	    .description = "Decode last 4-byte character",
	    .bytes = { 0xF4, 0x8F, 0xBF, 0xBF },
	    .expected = {
		.num_bytes = 4,
		.codepoint = 0x10FFFF,
	    },
	},
	{
	    .description = "Decode last codepoint before utf-16 surrogates",
	    .bytes = { 0xED, 0x9F, 0xBF },
	    .expected = {
		.num_bytes = 3,
		.codepoint = 0xD7FF,
	    },
	},
	{
	    .description = "Decode first codepoint after utf-16 surrogates",
	    .bytes = { 0xEE, 0x80, 0x80 },
	    .expected = {
		.num_bytes = 3,
		.codepoint = 0xE000,
	    },
	},
    };

    for (size_t ix = 0; ix < sizeof(tests) / sizeof(tests[0]); ++ix)
    {
        phbase_codepoint c = 0;
        size_t bytes = phbase_utf8_decode_character(&c, 6, tests[ix].bytes);

        if (bytes == 0)
        {
            PHBASE_TEST_ERROR(ctx, "%s: Failed to decode bytes", tests[ix].description);
            continue;
        }

        if (bytes != tests[ix].expected.num_bytes)
        {
            PHBASE_TEST_ERROR(ctx, "%s: Expected to decoded %ld bytes, but expected %ld",
                         tests[ix].description, bytes, tests[ix].expected.num_bytes);
        }

        if (c != tests[ix].expected.codepoint)
        {
            PHBASE_TEST_ERROR(ctx, "%s: Got codepoint %d (%lc), but expected %d (%lc)",
                         tests[ix].description, c, c, tests[ix].expected.codepoint,
			      tests[ix].expected.codepoint);
        }
    }
}

static void
test_inside_surrogate_pair_boundaries(phbase_test_ctx* ctx)
{
    // Test lower end of surrogate range
    {
        const uint8_t lower[] = { 0xED, 0xA0, 0x80 };
        phbase_codepoint c = 0;

        size_t lower_bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(lower), lower);

        if (lower_bytes != 0)
        {
            PHBASE_TEST_ERROR(ctx, "Expected decode failure on lower bound of surrogate pairs, "
			      "but decoded %ld byte%s to obtain '%lc'",
			      lower_bytes, lower_bytes == 1 ? "" : "s", c);
        }
    }

    // Test upper end of surrogate range
    {
        const uint8_t upper[] = { 0xED, 0xBF, 0xBF };
        phbase_codepoint c = 0;

        size_t upper_bytes = phbase_utf8_decode_character(&c, PHBASE_STATIC_ARRAY_LENGTH(upper), upper);

        if (upper_bytes != 0)
        {
            PHBASE_TEST_ERROR(ctx, "Expected decode failure on upper bound of surrogate pairs, "
			      "but decoded %ld byte%s to obtain '%lc'",
			      upper_bytes, upper_bytes == 1 ? "" : "s", c);
        }
    }
}

static struct phbase_test tests[] = {
    PHBASE_TEST_RECORD(test_decode_ascii),
    PHBASE_TEST_RECORD(test_decode_empty),
    PHBASE_TEST_RECORD(test_decode_lambda_character),
    PHBASE_TEST_RECORD(test_decode_bad_continuation),
    PHBASE_TEST_RECORD(test_decode_bad_initial_byte),
    PHBASE_TEST_RECORD(test_decode_long_encoded_ascii),
    PHBASE_TEST_RECORD(test_character_length_boundaries),
    PHBASE_TEST_RECORD(test_inside_surrogate_pair_boundaries),
};

static struct phbase_test_testsuite suite = {
    .name = "phbase/utf8",
    .length = PHBASE_STATIC_ARRAY_LENGTH(tests),
    .tests = tests
};

const struct phbase_test_testsuite* phbase_utf8_suite = &suite;
