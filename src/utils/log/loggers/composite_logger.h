#ifndef __UTILS_LOG_LOGGERS_COMPOSITE_LOGGER_H__
#define __UTILS_LOG_LOGGERS_COMPOSITE_LOGGER_H__

#include <utils/log/logger.h>

#include <memory>
#include <vector>

namespace Utils
{
  namespace Log
  {
    std::unique_ptr<Logger> CreateCompositeLogger(
      std::vector<std::unique_ptr<Logger>>&& loggers);
  }
}

#endif
