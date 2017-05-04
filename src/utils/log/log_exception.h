#ifndef __UTILS_LOG_LOG_EXCEPTION_H__
#define __UTILS_LOG_LOG_EXCEPTION_H__

#include <exception>
#include <string>

namespace Utils
{
  namespace Log
  {
    class Logger;

    void LogException(Logger& logger, std::exception_ptr exception, const std::string& context = std::string());
    void LogCurrentException(Logger& logger, const std::string& context = std::string());
    void LogException(std::exception_ptr exception, const std::string& context = std::string());
    void LogCurrentException(const std::string& context = std::string());
  }
}

#endif
