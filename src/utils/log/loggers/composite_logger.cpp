#include <utils/log/loggers/composite_logger.h>
#include <utils/err.h>

namespace Utils
{
namespace Log
{
  namespace
  {
    class CompositeLogger : public Logger
    {
      std::vector<std::unique_ptr<Logger>> m_loggers;

    public:
      explicit CompositeLogger(std::vector<std::unique_ptr<Logger>>&& loggers)
        : m_loggers(std::move(loggers))
      {
        CheckNoNullLoggers();
      }

      virtual void Add(const char* log, Level level) override
      {
        ForEach([log, level](Logger& logger)
        {
          logger.Add(log, level);
        });
      }

      virtual void Add(const char* log, Level level, const char* meta, unsigned indent) override
      {
        ForEach([log, level, meta, indent](Logger& logger)
        {
          logger.Add(log, level, meta, indent);
        });
      }

      virtual void PushIndent() override
      {
        ForEach([](Logger& logger)
        {
          logger.PushIndent();
        });
      }

      virtual void PopIndent() override
      {
        ForEach([](Logger& logger)
        {
          logger.PopIndent();
        });
      }

    private:
      void CheckNoNullLoggers() const
      {
        for (const auto& logger : m_loggers)
        {
          ERR_THROW_IF(logger.get() == nullptr, "Null logger.");
        }
      }

      template <typename F>
      void ForEach(F func)
      {
        for (auto& logger : m_loggers)
        {
          func(*logger);
        }
      }

      CompositeLogger(const CompositeLogger&) = delete;
      CompositeLogger& operator = (const CompositeLogger&) = delete;
    };
  }

  std::unique_ptr<Logger> CreateCompositeLogger(
    std::vector<std::unique_ptr<Logger>>&& loggers)
  {
    return std::make_unique<CompositeLogger>(std::move(loggers));
  }
}
}
