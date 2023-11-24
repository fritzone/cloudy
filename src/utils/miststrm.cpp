#include "miststrm.h"

MyIStringStream::MyIStringStream(const std::string &str) : buffer(str), position(0) {}

MyIStringStream &MyIStringStream::operator>>(int &value)
{
    readInteger(value);
    return *this;
}

MyIStringStream &MyIStringStream::operator>>(double &value)
{
    readDouble(value);
    return *this;
}

MyIStringStream &MyIStringStream::operator>>(std::string &value)
{
    readString(value);
    return *this;
}

MyIStringStream &MyIStringStream::operator>>(bool &value)
{
    readBoolean(value);
    return *this;
}

void MyIStringStream::readInteger(int &value)
{
    while (position < buffer.size() && std::isspace(buffer[position]))
    {
        ++position;
    }

    int result = 0;
    bool isNegative = false;

    if (buffer[position] == '-')
    {
        isNegative = true;
        ++position;
    }

    while (position < buffer.size() && std::isdigit(buffer[position]))
    {
        result = result * 10 + (buffer[position] - '0');
        ++position;
    }

    value = isNegative ? -result : result;
}

void MyIStringStream::readDouble(double &value)
{
    while (position < buffer.size() && std::isspace(buffer[position]))
    {
        ++position;
    }

    double result = 0.0;
    bool isNegative = false;

    if (buffer[position] == '-')
    {
        isNegative = true;
        ++position;
    }

    while (position < buffer.size() && (std::isdigit(buffer[position]) || buffer[position] == '.'))
    {
        if (std::isdigit(buffer[position]))
        {
            result = result * 10 + (buffer[position] - '0');
        }
        else
            if (buffer[position] == '.')
            {
                ++position;
                double decimal = 0.1;

                while (position < buffer.size() && std::isdigit(buffer[position]))
                {
                    result = result + (buffer[position] - '0') * decimal;
                    decimal /= 10.0;
                    ++position;
                }
                break;
            }
        ++position;
    }
    value = isNegative ? -result : result;
}

void MyIStringStream::readString(std::string &value)
{
    while (position < buffer.size() && std::isspace(buffer[position]))
    {
        ++position;
    }

    std::size_t start = position;
    while (position < buffer.size() && !std::isspace(buffer[position]))
    {
        ++position;
    }

    value = buffer.substr(start, position - start);
}

void MyIStringStream::readBoolean(bool &value)
{
    while (position < buffer.size() && std::isspace(buffer[position]))
    {
        ++position;
    }

    if (buffer.substr(position, 4) == "true")
    {
        value = true;
        position += 4;
    }
    else
        if (buffer.substr(position, 5) == "false")
        {
            value = false;
            position += 5;
        }
}
