#ifndef _BASE64_H_
#define _BASE64_H_

#include <stddef.h>

/**
 * How many characters the base64 form of len bytes needs, without the '\0'
 */
#define BASE64_ENCODED_LEN(len) ((((len) + 2) / 3) * 4)

/**
 * Encodes len bytes from in to out, which must hold BASE64_ENCODED_LEN(len) + 1
 * characters. The output is '\0' terminated. Returns the length of the output.
 */
size_t base64_encode(const unsigned char* in, size_t len, char* out);

/**
 * Decodes the '\0' terminated base64 text into out, which can hold outSize bytes.
 * Characters which are not part of the base64 alphabet are skipped.
 * Returns the number of decoded bytes, or -1 if out is too small.
 */
long base64_decode(const char* in, unsigned char* out, size_t outSize);

#endif
