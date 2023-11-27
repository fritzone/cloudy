#ifndef _LOG_TYPES_H
#define _LOG_TYPES_H

#include <map>
#include <string>

enum LogLevel
{
    LOG_EMERGENCY   = 1,
    LOG_CRITICAL    = 2,
    LOG_ERROR       = 3,
    LOG_WARNING     = 4,
    LOG_INFORMATION = 5,
    LOG_DEBUG       = 6,
    LOG_TRACE       = 7
};

static std::map<LogLevel, std::string> createLogLevelMap()
{
    std::map<LogLevel, std::string> errstrings;
    errstrings[LOG_EMERGENCY] = "EMERGENCY   ";
    errstrings[LOG_CRITICAL] = "CRITICAL    ";
    errstrings[LOG_ERROR] = "ERROR       ";
    errstrings[LOG_WARNING] = "WARNING     ";
    errstrings[LOG_INFORMATION] = "INFORMATION ";
    errstrings[LOG_DEBUG] = "DEBUG       ";
    errstrings[LOG_TRACE] = "TRACE       ";

    return errstrings;
}

static std::map<LogLevel, std::string> errstrings = createLogLevelMap();


#endif
