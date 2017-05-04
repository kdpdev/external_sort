#ifndef __UTILS_LOG_LOGGERS_OSTREAM_LOGGER_H__
#define __UTILS_LOG_LOGGERS_OSTREAM_LOGGER_H__

#include <utils/log/logger.h>

#include <iosfwd>
#include <memory>
#include <string>

namespace Utils
{
  namespace Log
  {
    std::unique_ptr<Logger> CreateOutStreamLogger(std::ostream& out);
    std::unique_ptr<Logger> CreateOutStreamLogger(std::unique_ptr<std::ostream> out);

    std::unique_ptr<Logger> CreateCoutLogger();
    std::unique_ptr<Logger> CreateFoutLogger(const std::string& filePath, bool truncateFile = false);
  }
}

#endif
