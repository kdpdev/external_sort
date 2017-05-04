#ifndef __SCOPE_TIME_LOGGER_H__
#define __SCOPE_TIME_LOGGER_H__

#include <chrono>
#include <string>

namespace Utils
{
  class ScopeTimeLogger
  {
  public:
    using Clock = std::chrono::system_clock;

  private:
    const std::string m_scopeName;
    Clock::time_point m_startTime;

  public:
    explicit ScopeTimeLogger(const std::string& scopeName);
    ~ScopeTimeLogger();

    Clock::duration GetDuration() const;

    ScopeTimeLogger(const ScopeTimeLogger&) = delete;
    ScopeTimeLogger& operator = (const ScopeTimeLogger&) = delete;
  };
}

#endif
