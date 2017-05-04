#ifndef __UTILS_LOG_LOGGERS_THREADSAFE_LOGGER_H__
#define __UTILS_LOG_LOGGERS_THREADSAFE_LOGGER_H__

#include <utils/log/logger.h>

#include <memory>

namespace Utils
{
  namespace Log
  {
    std::unique_ptr<Logger> CreateThreadsafeAsyncLogger(std::unique_ptr<Logger> logger);
    std::unique_ptr<Logger> CreateThreadsafeSyncLogger(std::unique_ptr<Logger> logger);
  }
}

#endif
