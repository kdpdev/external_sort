#ifndef __UTILS_LOG_LOG_H__
#define __UTILS_LOG_LOG_H__

#include <utils/log/logger.h>
#include <utils/log/log_exception.h>
#include <utils/log/scoped_log.h>

#include <string>

namespace Utils
{
  namespace Log
  {
    void LogData(Level level, const char* fmt, ...);
    void LogString(Level level, const char* str);
    void LogString(Level level, const std::string& str);
  }
}

#define LOG_JOIN(X, Y) LOG_JOIN_IMPL(X, Y)
#define LOG_JOIN_IMPL(X, Y) X##Y


#define LOG_SCOPE_D(name) const ::Utils::Log::ScopedDebugLog LOG_JOIN(scopedLog, __LINE__)(name)
#define LOG_FN_D LOG_SCOPE_D(__FUNCTION__)

#define LOG_SCOPE_I(name) const ::Utils::Log::ScopedInfoLog LOG_JOIN(scopedLog, __LINE__)(name)
#define LOG_FN_I LOG_SCOPE_I(__FUNCTION__)

#define LOG_SCOPE_W(name) const ::Utils::Log::ScopedWarningLog LOG_JOIN(scopedLog, __LINE__)(name)
#define LOG_FN_W LOG_SCOPE_W(__FUNCTION__)

#define LOG_SCOPE_E(name) const ::Utils::Log::ScopedErrorLog LOG_JOIN(scopedLog, __LINE__)(name)
#define LOG_FN_E LOG_SCOPE_E(__FUNCTION__)

#define LOG_SCOPE(name) LOG_SCOPE_D(name)
#define LOG_FN LOG_SCOPE(__FUNCTION__)


#define LOG_E(...) ::Utils::Log::LogData(::Utils::Log::LOG_LEVEL_ERROR,   __VA_ARGS__)
#define LOG_W(...) ::Utils::Log::LogData(::Utils::Log::LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_I(...) ::Utils::Log::LogData(::Utils::Log::LOG_LEVEL_INFO,    __VA_ARGS__)
#define LOG_D(...) ::Utils::Log::LogData(::Utils::Log::LOG_LEVEL_DEBUG,   __VA_ARGS__)

#endif
