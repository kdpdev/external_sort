#include <utils/log/loggers/null_logger.h>

namespace Utils
{
namespace Log
{
  namespace
  {
    class NullLogger : public Logger
    {
    public:
      NullLogger()
      {
      }

      virtual void Add(const char*, Level) override
      {
      }

      virtual void Add(const char*, Level, const char*, unsigned) override
      {
      }

      virtual void PushIndent() override
      {
      }

      virtual void PopIndent() override
      {
      }

      NullLogger(const NullLogger&) = delete;
      NullLogger& operator = (const NullLogger&) = delete;
    };
  }

  std::unique_ptr<Logger> CreateNullLogger()
  {
    return std::make_unique<NullLogger>();
  }
}
}
