#include <utils/log/scoped_log.h>
#include <utils/log/log_registry.h>

#include <exception>

namespace Utils
{
namespace Log
{
  ScopedLog::ScopedLog(Level level, const std::string& name, Logger* logger)
    : m_level(level)
    , m_name(name)
    , m_logger(logger)
  {
    Logger& log = m_logger ? *m_logger : GetLogger();
    log.Add(("+ " + m_name).c_str(), m_level);
    log.PushIndent();
  }

  ScopedLog::~ScopedLog()
  {
    Logger& logger = m_logger ? *m_logger : GetLogger();
    logger.PopIndent();
    const char* prefix = std::uncaught_exception() ? "!-" : "- ";
    logger.Add((prefix + m_name).c_str(), m_level);
  }
}
}
