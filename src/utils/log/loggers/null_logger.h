#ifndef __UTILS_LOG_LOGGERS_NULL_LOGGER_H__
#define __UTILS_LOG_LOGGERS_NULL_LOGGER_H__

#include <utils/log/logger.h>

#include <memory>

namespace Utils
{
namespace Log
{
  std::unique_ptr<Logger> CreateNullLogger();
}
}

#endif
