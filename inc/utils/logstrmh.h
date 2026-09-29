#ifndef _LOGSTREAMHELPER_H_
#define _LOGSTREAMHELPER_H_

#include "logtypes.h"

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

// When the level is disabled nothing after the << is evaluated
#define LOG_AT(level) if((level) > g_logLevel) ; else logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, level)

#define log_emergency()     LOG_AT(LOG_EMERGENCY)
#define log_critical()      LOG_AT(LOG_CRITICAL)
#define log_error()         LOG_AT(LOG_ERROR)
#define log_warning()       LOG_AT(LOG_WARNING)
#define log_info()          LOG_AT(LOG_INFORMATION)
#define log_debug()         LOG_AT(LOG_DEBUG)
#define log_trace()         LOG_AT(LOG_TRACE)

#endif  // LOGSTREAMHELPER_H
