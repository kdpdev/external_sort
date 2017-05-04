#define _CRT_SECURE_NO_WARNINGS

#include <utils/log/log.h>
#include <utils/log/log_registry.h>

#include <cstdio>
#include <cstdarg>

namespace Utils
{
namespace Log
{
  void LogData(Level level, const char* fmt, ...)
  {
    char buffer[1024] = { 0 };
    char* const  buffPtr = &buffer[0];
    constexpr std::size_t buffSize = (sizeof(buffer) / sizeof(*buffer)) - 1;

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffPtr, buffSize, fmt, args);
    va_end(args);
    
    Log::GetLogger().Add(buffPtr, level);
  }

  void LogString(Level level, const char* str)
  {
    Log::GetLogger().Add(str, level);
  }

  void LogString(Level level, const std::string& str)
  {
    LogString(level, str.c_str());
  }
}
}
