#ifndef _LOGSTREAM_H_
#define _LOGSTREAM_H_

#include "logtypes.h"
#include <mstrstrm.h>

#include <sstream>
#include <string>

class logstream
{
public:

    logstream(int line, const char *pFile, const char *pFunc, LogLevel level);

    virtual ~logstream();

    logstream& operator<<(bool t);
    logstream& operator<<(char t);
    logstream& operator<<(signed short t);
    logstream& operator<<(unsigned short t);
    logstream& operator<<(signed int t);
    logstream& operator<<(unsigned int t);
    logstream& operator<<(signed long t);
    logstream& operator<<(unsigned long t);
    logstream& operator<<(float t);
    logstream& operator<<(double t);
    logstream& operator<<(const char *pChars);
    logstream& operator<<(const std::string &rString);
    logstream& operator<<(const void *pData);

private:
    logstream();
    logstream &operator=(const logstream &rOther);
    logstream(const logstream &rOther);

    inline logstream &appendSpace();

private:
    std::string mStreamBuffer;
    MyStringStream mOutputStream;
    std::string mFile;
    std::string mFunc;
    LogLevel mLevel;
    int mLine;
};

#endif  // logstream_H
