#ifndef _MYISTRINGSTREAM_H_
#define _MYISTRINGSTREAM_H_

#include <string>

/**
 * @brief The MyIStringStream class is a dummy implementetion of
 * the std::istringstream class, since watcom does not support it
 */
class MyIStringStream
{
public:
    MyIStringStream(const std::string& str);
    MyIStringStream& operator>>(int& value);
    MyIStringStream& operator>>(double& value);
    MyIStringStream& operator>>(std::string& value);
    MyIStringStream& operator>>(bool& value);

private:
    void readInteger(int& value);
    void readDouble(double& value);
    void readString(std::string& value);
    void readBoolean(bool& value);

private:
    std::string buffer;
    std::size_t position;
};

#endif
