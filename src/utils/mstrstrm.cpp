#include "mstrstrm.h"

MyStringStream::MyStringStream()
{
    buffer = "";
}

const char *MyStringStream::str() const
{
    return buffer.c_str();
}


MyStringStream &MyStringStream::operator<<(unsigned int value)
{
    append((int)value);
    return *this;
}

MyStringStream &MyStringStream::operator<<(int value)
{
    append(value);
    return *this;
}

MyStringStream &MyStringStream::operator<<(char value)
{
    buffer += value;
    return *this;
}

MyStringStream &MyStringStream::operator<<(const char *value)
{
    buffer += value;
    return *this;
}

MyStringStream &MyStringStream::operator<<(const std::string &value) {
    buffer += value;
    return *this;
}

MyStringStream &MyStringStream::operator<<(bool value)
{
    buffer += value?"true":"false";
    return *this;
}

MyStringStream &MyStringStream::operator<<(long value)
{
    append(value);
    return *this;
}

MyStringStream &MyStringStream::operator<<(unsigned long value)
{
    append((long)value);
    return *this;
}

MyStringStream &MyStringStream::operator<<(double value) {
    append(value);
    return *this;
}

MyStringStream &MyStringStream::operator<<(float value) {
    append(value);
    return *this;
}
