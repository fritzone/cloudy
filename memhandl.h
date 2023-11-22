#ifndef _MEMHANDLER_H_
#define _MEMHANDLER_H_

#include <types.h>

template<class T> T* allocate(uint32_t count = 1)
{
   return (T*)calloc(count, sizeof(T));
}

template<class T> void deallocate(T* pointer)
{
  free(pointer);
}

#endif

