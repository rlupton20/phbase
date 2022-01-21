#include <phbase/utf8.h>

#define UTF8_ASCII_MASK                  0x80  /* 0b10000000 */
#define UTF8_CONTINUATION_CODEPOINT_MASK 0x3F  /* 0b00111111 */
#define UTF8_MAX_CODEPOINT               0x10FFFF

#define UTF16_SURROGATE_MIN 0xD800
#define UTF16_SURROGATE_MAX 0xDFFF

// UTF-8 was initially described as a 6-byte encoding in RF2279 (section 2)
// The subsequent RFC3629 (section 3) restricted this to a 4-byte encoding,
// for compatiblity with UTF-16.

static const size_t utf8_length_by_initial[UINT8_MAX + 1] = {
    // 0 - 127 (ascii characters)
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    // 128 - 191 (these are the continuation bytes)
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    // 192 - 223
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 224 - 239
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,

    // 240 - 247
    4, 4, 4, 4, 4, 4, 4, 4,

    // Encoded characters use at most 4-bytes (since RFC3629), so return 0 on the
    // remaining leading bytes, so that we return an error (0 bytes read).
    // 248 - 251
    0, 0, 0, 0,

    // 252 - 253
    0, 0,

    // 254 - 255
    0 , 0
};

static const uint8_t utf8_codepoint_mask[UINT8_MAX + 1] = {
    // 0 - 127 (ascii characters)
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,

    // 128 - 191 (these are the continuation bytes) - extract bottom 6 bits
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,

    // 192 - 223 - extract bottom 5 bits
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,

    // 224 - 239 - extract bottom 4 bits
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,

    // 240 - 247 - extract bottom 3 bits
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,

    // 248 - 251 - extract bottom 2 bits
    0x03, 0x03, 0x03, 0x03,

    // 252 - 253 - extract bottom bit
    0x01, 0x01,

    // 254 - 255 (note: characters are at most 6-bytes)
    0x0, 0x0  // Extract no bits - this is invalid utf-8
};

size_t
phbase_utf8_decode_character(phbase_codepoint* out, size_t length, const uint8_t* bytes)
{
    if (length == 0)
    {
        return 0;
    }

    if (!(*bytes & UTF8_ASCII_MASK))
    {
        *out = *bytes;
        return 1;
    }

    phbase_codepoint c = *bytes & utf8_codepoint_mask[*bytes];
    size_t num_bytes = utf8_length_by_initial[*bytes];

    if (length < num_bytes)
    {
        return 0;
    }

    // valid remains 1 so long as all continuation bytes are valid
    uint8_t valid = 1;

    for (size_t ix = 1; ix < num_bytes; ++ix)
    {
        valid = valid * !((bytes[ix] & 0xC0) ^ 0x80);
        c = (c << 6) | (bytes[ix] & UTF8_CONTINUATION_CODEPOINT_MASK);
    }

    // Check that the extracted codepoint couldn't be packed into fewer bytes
    static const phbase_codepoint max_prior_codepoint[7] = {
        [0] = 0,
        [1] = 0,
        [2] = 127,    // > 7-bits
        [3] = 2047,   // > (5 + 6) bits
        [4] = 65535,  // > (4 + 6 + 6) bits
        // These are not used, because RFC3629 (see section 3) limits UTF-8 to 4
        // bytes. We put what they would have been, in case this needs adapting
        // to pre-RFC3629 UTF-8.
        [5] = 2097151,          // > (3 + 6 + 6 + 6) bits
        [6] = 1048576 * 64 - 1  // > (2 + 6 + 6 + 6 + 6) bits
    };

    // Check the encoding didn't use excessive bytes
    valid = valid * (max_prior_codepoint[num_bytes] < c);

    // Check we didn't exceed the maximum codepoint on a 4-byte read (RFC3629)
    valid = valid * (c < UTF8_MAX_CODEPOINT + 1);

    // Check for UTF-16 surrogate pairs (RFC3629 again)
    valid = valid * ((c < UTF16_SURROGATE_MIN) + (c > UTF16_SURROGATE_MAX));

    *out = c * valid;
    return num_bytes * valid;
}
