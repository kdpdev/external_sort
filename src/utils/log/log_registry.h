#ifndef __UTILS_LOG_LOG_REGISTRY_H__
#define __UTILS_LOG_LOG_REGISTRY_H__

#include <utils/log/logger.h>

#include <memory>

namespace Utils
{
  namespace Log
  {
    Logger& GetLogger();
    void SetLogger(std::unique_ptr<Logger> logger);
  }
}

#endif
