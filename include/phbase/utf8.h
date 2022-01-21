#ifndef PHBASE_UTF8_H
#define PHBASE_UTF8_H

#include <stdint.h>
#include <stddef.h>

/******************************************************************************/
/* Type for unicode codepoints.                                               */
/******************************************************************************/
typedef uint32_t phbase_codepoint;

/******************************************************************************/
/* Try and decode one codepoint from an array of characters.                  */
/******************************************************************************/
size_t
phbase_utf8_decode_character(phbase_codepoint* out, size_t length, const uint8_t* bytes);

#endif
