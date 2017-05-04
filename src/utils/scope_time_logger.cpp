#include <utils/scope_time_logger.h>
#include <utils/log/log.h>

namespace Utils
{
  ScopeTimeLogger::ScopeTimeLogger(const std::string& scopeName)
    : m_scopeName(scopeName)
    , m_startTime(Clock::now())
  {
  }

  ScopeTimeLogger::~ScopeTimeLogger()
  {
    const auto now = Clock::now();
    const auto diff = (now - m_startTime).count() / 1000;
    LOG_I("%s time: %s", m_scopeName.c_str(), std::to_string(diff).c_str());
  }

  ScopeTimeLogger::Clock::duration ScopeTimeLogger::GetDuration() const
  {
    return Clock::now() - m_startTime;
  }
}
