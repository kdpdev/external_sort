#include <utils/log/log_exception.h>
#include <utils/log/log_registry.h>
#include <utils/log/logger.h>
#include <utils/err.h>

#include <sstream>

namespace Utils
{
namespace Log
{
  void LogException(Logger& logger, std::exception_ptr exception, const std::string& context)
  {
    if (context.empty())
    {
      logger.Add(Utils::FormatException(exception).c_str(), Log::LOG_LEVEL_ERROR);
    }
    else
    {
      std::ostringstream oss;
      oss << context << "\r\n" << Utils::FormatException(exception);
      logger.Add(oss.str().c_str(), Log::LOG_LEVEL_ERROR);
    }
  }

  void LogCurrentException(Logger& logger, const std::string& context)
  {
    LogException(logger, std::current_exception(), context);
  }

  void LogException(std::exception_ptr exception, const std::string& context)
  {
    LogException(GetLogger(), exception, context);
  }

  void LogCurrentException(const std::string& context)
  {
    LogException(std::current_exception(), context);
  }
}
}
