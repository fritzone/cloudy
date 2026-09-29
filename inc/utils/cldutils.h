#ifndef UTILS_H
#define UTILS_H

#include <types.h>

/**
 * @brief rand_string will generate a random ascii string of the specified size
 * @return char pointer, does not need to be freed
 */
const char* __far rand_string(size_t size);

/**
 * @brief Renders human readalbe( kb, mb, ...) size of the input number
 * @return char pointer, does not need to be freed
 */
const char* __far renderHumanReadableSize(unsigned long);

/**
 * @brief Renders human readable size of the input number which is in kilobytes
 * @return char pointer, does not need to be freed
 */
const char* __far renderHumanReadableSizeKB(unsigned long);

/**
 * @brief Turns a long file name into an uppercase DOS 8.3 name
 * @param out must hold at least 13 characters
 */
void makeDosName(const char* longName, char* out);

#endif // UTILS_H
