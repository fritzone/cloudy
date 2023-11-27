#ifndef _LOGSTREAMHELPER_H_
#define _LOGSTREAMHELPER_H_

#include "logtypes.h"

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define log_emergency()     logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_EMERGENCY)
#define log_critical()      logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_CRITICAL)
#define log_error()         logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_ERROR)
#define log_warning()       logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_WARNING)
#define log_info()          logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_INFORMATION)
#define log_debug()         logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_DEBUG)
#define log_trace()         logstream(__LINE__, __FILE__, __PRETTY_FUNCTION__, LOG_TRACE)

#endif  // LOGSTREAMHELPER_H
