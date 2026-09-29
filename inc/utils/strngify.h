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

/**
 * Escapes the characters which cannot appear as they are in XML text
 */
inline std::string xml_escape(const std::string& s)
{
    // most of the time there is nothing to escape (file data is base64)
    if(s.find_first_of("&<>\"") == std::string::npos)
    {
        return s;
    }

    std::string result;
    result.reserve(s.length());
    for(size_t i = 0; i < s.length(); i++)
    {
        switch(s[i])
        {
        case '&': result += "&amp;"; break;
        case '<': result += "&lt;"; break;
        case '>': result += "&gt;"; break;
        case '"': result += "&quot;"; break;
        default: result += s[i];
        }
    }
    return result;
}

/**
 * Reads a bool written either as 1/0 or true/false
 */
inline bool xml_to_bool(const char* s)
{
    return s && (s[0] == '1' || s[0] == 't');
}

#endif
