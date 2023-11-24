#ifndef _STRNGFY_H_
#define _STRNGFY_H_

#include "mstrstrm.h"
#include "miststrm.h"

#include <string>

template <typename T>
std::string stringify(const T& value)
{
    MyStringStream oss;
    oss << value;
    return oss.str();
}

template <typename T>
void destringify(T& value, const std::string& str)
{
    MyIStringStream iss(str);
    iss >> value;
}

#endif
