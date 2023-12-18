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


#endif // UTILS_H
