#ifndef _MYSTRINGSTREAM_H_
#define _MYSTRINGSTREAM_H_

#include <string>

class MyStringStream
{
public:

    MyStringStream();

    const char* str() const;

    MyStringStream& operator<<(int value);
    MyStringStream& operator<<(char value);
    MyStringStream& operator<<(const char* value);
    MyStringStream& operator<<(const std::string& value);
    MyStringStream& operator<<(bool value);

private:
    void appendInteger(int value);

private:
    std::string buffer;
};

#endif
