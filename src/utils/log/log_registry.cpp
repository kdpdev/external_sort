#include <utils/log/log_registry.h>
#include <utils/log/loggers/ostream_logger.h>
#include <utils/err.h>

#include <stdexcept>

namespace Utils
{
namespace Log
{
  namespace
  {
    std::unique_ptr<Logger>& GetLoggerHolder()
    {
      static std::unique_ptr<Logger> logger(std::move(CreateCoutLogger()));
      return logger;
    }
  }

  Logger& GetLogger()
  {
    return *GetLoggerHolder();
  }

  void SetLogger(std::unique_ptr<Logger> logger)
  {
    ERR_THROW_TYPED_IF(logger.get() == nullptr, std::invalid_argument, "Null logger.");
    GetLoggerHolder() = std::move(logger);
  }
}
}
