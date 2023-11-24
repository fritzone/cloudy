#include "mstrstrm.h"

MyStringStream::MyStringStream()
{
    buffer = "";
}

const char *MyStringStream::str() const
{
    return buffer.c_str();
}

MyStringStream &MyStringStream::operator<<(int value)
{
    appendInteger(value);
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

void MyStringStream::appendInteger(int value)
{
    if (value < 0)
    {
        buffer += '-';
        value = -value;
    }
    int divisor = 1;
    while (divisor <= value / 10)
    {
        divisor *= 10;
    }
    while (divisor > 0)
    {
        int digit = value / divisor;
        buffer += '0' + digit;
        value %= divisor;
        divisor /= 10;
    }
}
