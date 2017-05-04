#ifndef __UTILS_LOG_SCOPED_LOG_H__
#define __UTILS_LOG_SCOPED_LOG_H__

#include <utils/log/log_level.h>

#include <string>

namespace Utils
{
  namespace Log
  {
    class Logger;

    class ScopedLog
    {
      const Level m_level;
      const std::string m_name;
      Logger* const m_logger;

    public:
      ScopedLog(Level level, const std::string& name, Logger* logger = nullptr);
      virtual ~ScopedLog();

      ScopedLog(const ScopedLog&) = delete;
      ScopedLog& operator = (const ScopedLog&) = delete;
    };

    template <Level L>
    class ScopedLogTemplate : public ScopedLog
    {
    public:
      explicit ScopedLogTemplate(const std::string& name, Logger* logger = nullptr)
        : ScopedLog(L, name, logger)
      {
      }
    };

    using ScopedErrorLog   = ScopedLogTemplate<LOG_LEVEL_ERROR>;
    using ScopedWarningLog = ScopedLogTemplate<LOG_LEVEL_WARNING>;
    using ScopedInfoLog    = ScopedLogTemplate<LOG_LEVEL_INFO>;
    using ScopedDebugLog   = ScopedLogTemplate<LOG_LEVEL_DEBUG>;
  }
}

#endif
