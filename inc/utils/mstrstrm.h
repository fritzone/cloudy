#ifndef _MYSTRINGSTREAM_H_
#define _MYSTRINGSTREAM_H_

#include <string>
#include <stdio.h>

class MyStringStream
{
public:

    MyStringStream();

    const char* str() const;

    MyStringStream& operator<<(int value);
    MyStringStream& operator<<(long value);
    MyStringStream& operator<<(unsigned long value);
    MyStringStream& operator<<(unsigned int value);

    MyStringStream& operator<<(void* value);

    // Overloaded operator<< for doubles
    MyStringStream& operator<<(double value);

    // Overloaded operator<< for floats
    MyStringStream& operator<<(float value);

    MyStringStream& operator<<(char value);
    MyStringStream& operator<<(const char* value);
    MyStringStream& operator<<(const std::string& value);
    MyStringStream& operator<<(bool value);

    void flush() {};

private:
    // Helper function to append an integer to the buffer
    void append(int value) {
        // Convert the integer to string and append
        char temp[32];
        snprintf(temp, sizeof(temp), "%d", value);
        buffer += temp;
    }

    // Helper function to append a double to the buffer
    void append(double value) {
        // Convert the double to string and append
        char temp[32];
        snprintf(temp, sizeof(temp), "%.6f", value);
        buffer += temp;
    }

    // Helper function to append a float to the buffer
    void append(float value) {
        // Convert the float to string and append
        char temp[32];
        snprintf(temp, sizeof(temp), "%.6f", value);
        buffer += temp;
    }

    //lper function to append a long to the buffer
    void append(long value) {
        // Convert the long to string and append
        char temp[32];
        snprintf(temp, sizeof(temp), "%ld", value);
        buffer += temp;
    }


    //Helper function to append a pointer to the buffer
    void append(void* value) {
        // Convert the long to string and append
        char temp[32];
        snprintf(temp, sizeof(temp), "0x%p", value);
        buffer += temp;
    }

private:
    std::string buffer;
};

#endif
