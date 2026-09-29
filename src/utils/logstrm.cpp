#include "logstrm.h"

#include <stdio.h>
#include <time.h>

LogLevel g_logLevel = LOG_WARNING;

const char* logLevelName(LogLevel level)
{
    switch(level)
    {
    case LOG_EMERGENCY: return "EMERGENCY   ";
    case LOG_CRITICAL: return "CRITICAL    ";
    case LOG_ERROR: return "ERROR       ";
    case LOG_WARNING: return "WARNING     ";
    case LOG_INFORMATION: return "INFORMATION ";
    case LOG_DEBUG: return "DEBUG       ";
    case LOG_TRACE: return "TRACE       ";
    }
    return "            ";
}

logstream::logstream(int line, const char *pFile, const char *pFunc, LogLevel level) :
    mOutputStream(),
    mFile(pFile),
    mFunc(pFunc),
    mLevel(level),
    mLine(line)
{
}
// Definition of timeToStr function
std::string timeToStr(time_t &time)
{
    MyStringStream ss;
    const std::string format = "%Y-%m-%d %H:%M:%S";
    std::tm* tmInfo = std::localtime(&time); // Locale time-zone, usually UTC by default.
    static char buffer[128];
    if (std::strftime(buffer, sizeof(buffer), format.c_str(), tmInfo) != 0)
    {
        ss << buffer;
    }
    std::string ts = ss.str();
    return ts;
}
// Definition of do_log function
void do_log(const std::string& msg)
{
    // opened and closed every time, so the log survives a crash
    FILE* fs = fopen("clouder.log", "a");
    if(fs == NULL)
    {
        return;
    }

    std::time_t currentTime = std::time(NULL);
    fprintf(fs, "[%s]%s", timeToStr(currentTime).c_str(), msg.c_str());
    fclose(fs);
}

logstream::~logstream()
{
    MyStringStream arguments;

    arguments << logLevelName(mLevel)
              << mFile.substr(mFile.find_last_of('/') + 1)
              << ":" << mLine << " (" << mFunc << ")" << mOutputStream.str() << "\n";

    do_log(arguments.str());

}

logstream &logstream::appendSpace()
{
    mOutputStream << ' ';
    mOutputStream.flush();
    return *this;
}

logstream &logstream::operator<<(bool t)
{
    mOutputStream << (t ? "true" : "false");
    return appendSpace();
}

logstream &logstream::operator<<(char t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(signed short t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(unsigned short t)
{
    mOutputStream << (int)t;
    return appendSpace();
}

logstream &logstream::operator<<(signed int t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(unsigned int t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(signed long t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(unsigned long t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(float t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(double t)
{
    mOutputStream << t;
    return appendSpace();
}

logstream &logstream::operator<<(const char *pChars)
{
    mOutputStream << pChars;
    return appendSpace();
}

logstream &logstream::operator<<(const std::string &rString)
{
    mOutputStream << '\"' << rString << '\"';
    return appendSpace();
}

logstream &logstream::operator<<(const void *pData)
{
    mOutputStream << (void*)pData;
    return appendSpace();
}
