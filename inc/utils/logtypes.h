#ifndef _LOG_TYPES_H
#define _LOG_TYPES_H

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

/**
 * Messages above this level are not logged. Warnings and worse by default.
 */
extern LogLevel g_logLevel;

/**
 * The name of the level, as it appears in the log
 */
const char* logLevelName(LogLevel level);

#endif
