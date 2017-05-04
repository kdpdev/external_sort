#ifndef __UTILS_LOG_LOGGER_H__
#define __UTILS_LOG_LOGGER_H__

#include <utils/log/log_level.h>

#include <memory>
#include <string>

namespace Utils
{
namespace Log
{
  class Logger
  {
  public:
    virtual ~Logger() = default;

    virtual void Add(const char* log, Level level) = 0;
    virtual void Add(const char* log, Level level, const char* meta, unsigned indent) = 0;
    virtual void PushIndent() = 0;
    virtual void PopIndent() = 0;
  };
}
}

#endif
