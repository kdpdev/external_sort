#include <utils/log/loggers/threadsafe_logger.h>
#include <utils/err.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace Utils
{
namespace Log
{
  namespace
  {
    struct LogItem
    {
      std::chrono::system_clock::time_point timePoint;
      std::thread::id threadId;
      std::string meta;
      std::string text;
      Level level;

      LogItem()
        : timePoint(std::chrono::system_clock::now())
        , threadId(std::this_thread::get_id())
      {
      }

      LogItem(const char* theText, Level theLevel)
        : timePoint(std::chrono::system_clock::now())
        , threadId(std::this_thread::get_id())
        , text(theText ? theText : "(null)")
        , level(theLevel)
      {
      }

      LogItem(const char* theMeta, const char* theText, Level theLevel)
        : timePoint(std::chrono::system_clock::now())
        , threadId(std::this_thread::get_id())
        , meta(theMeta ? theMeta : "")
        , text(theText ? theText : "(null)")
        , level(theLevel)
      {
      }

      LogItem(LogItem&& other)
        : timePoint(std::move(other.timePoint))
        , threadId(std::move(other.threadId))
        , meta(std::move(other.meta))
        , text(std::move(other.text))
        , level(std::move(other.level))
      {
      }

      LogItem& operator = (LogItem&& other)
      {
        if (this != &other)
        {
          timePoint = std::move(other.timePoint);
          threadId = std::move(other.threadId);
          meta = std::move(other.meta);
          text = std::move(other.text);
          level = std::move(other.level);
        }
        return *this;
      }
    };

    void AddLogItem(Logger& logger, const LogItem& logItem, unsigned indent)
    {
      std::ostringstream oss;
      oss << '[' << std::chrono::system_clock::to_time_t(logItem.timePoint) << "] : ";
      oss << '[' << logItem.threadId << "] : ";
      if (!logItem.meta.empty())
      {
        oss << logItem.meta << " : ";
      }
      logger.Add(logItem.text.c_str(),
                 logItem.level,
                 oss.str().c_str(),
                 indent);
    }


    class ThreadsafeAsyncLogger : public Logger
    {
      struct AsyncLogItem : public LogItem
      {
        enum class Type
        {
          UNKNOWN,
          COMMAND,
          MESSAGE,
        };

        enum class Command
        {
          UNKNOWN,
          PUSH_INDENT,
          POP_INDENT,
        };

        Type type;
        Command command;

        AsyncLogItem(Type theType, Command theCommand)
          : LogItem()
          , type(theType)
          , command(theCommand)
        {
        }

        AsyncLogItem(const char* theText, Level theLevel, Type theType)
          : LogItem(theText, theLevel)
          , type(theType)
          , command(Command::UNKNOWN)
        {
        }

        AsyncLogItem(const char* theMeta, const char* theText, Level theLevel, Type theType)
          : LogItem(theMeta, theText, theLevel)
          , type(theType)
          , command(Command::UNKNOWN)
        {
        }

        AsyncLogItem(AsyncLogItem&& other)
          : LogItem(std::move(other))
          , type(other.type)
          , command(other.command)
        {
        }

        AsyncLogItem& operator = (AsyncLogItem&& other)
        {
          LogItem::operator = (std::move(other));
          if (this != &other)
          {
            threadId = std::move(other.threadId);
            type = std::move(other.type);
            command = std::move(other.command);
          }
          return *this;
        }
      };

      using LogItemQueue = std::queue<AsyncLogItem>;
      using ThreadIndentMap = std::map<std::thread::id, unsigned>;

      std::unique_ptr<Logger> m_logger;
      std::mutex m_pushQueueGuard;
      std::condition_variable m_wakeupEvent;
      std::unique_ptr<LogItemQueue> m_pushQueue;
      std::unique_ptr<LogItemQueue> m_popQueue;
      std::atomic_bool m_stopLogThread;
      std::thread m_logThread;
      ThreadIndentMap m_threadIndents;

    public:
      explicit ThreadsafeAsyncLogger(std::unique_ptr<Logger> logger)
        : m_logger(std::move(logger))
        , m_pushQueue(std::make_unique<LogItemQueue>())
        , m_popQueue(std::make_unique<LogItemQueue>())
        , m_stopLogThread(false)
      {
        ERR_THROW_IF(m_logger.get() == nullptr, "Null logger.");
        m_logThread = std::thread(&ThreadsafeAsyncLogger::LogThread, this);
      }

      virtual ~ThreadsafeAsyncLogger()
      {
        m_stopLogThread = true;
        m_wakeupEvent.notify_one();
        m_logThread.join();

        ProcessLogs(*m_popQueue);
        ProcessLogs(*m_pushQueue);
      }

      virtual void Add(const char* log, Level level) override
      {
        PushLogItem(AsyncLogItem(log, level, AsyncLogItem::Type::MESSAGE));
      }

      virtual void Add(const char* log, Level level, const char* meta, unsigned) override
      {
        PushLogItem(AsyncLogItem(meta, log, level, AsyncLogItem::Type::MESSAGE));
      }

      virtual void PushIndent() override
      {
        PushLogItem(AsyncLogItem(AsyncLogItem::Type::COMMAND, AsyncLogItem::Command::PUSH_INDENT));
      }

      virtual void PopIndent() override
      {
        PushLogItem(AsyncLogItem(AsyncLogItem::Type::COMMAND, AsyncLogItem::Command::POP_INDENT));
      }

    private:
      void PushLogItem(AsyncLogItem&& logItem)
      {
        std::lock_guard<std::mutex> guard(m_pushQueueGuard);
        m_pushQueue->push(std::move(logItem));
        m_wakeupEvent.notify_one();
      }

      void LogThread()
      {
        try
        {
          while (!m_stopLogThread)
          {
            ProcessLogs(*m_popQueue);

            if (m_popQueue->empty() && !m_stopLogThread)
            {
              std::unique_lock<std::mutex> guard(m_pushQueueGuard);
              m_wakeupEvent.wait_for(guard, std::chrono::seconds(5), [this] { return !m_pushQueue->empty() || m_stopLogThread; });
              // The m_popQueue is empty here. So the swap is correct event if the m_pushQueue is empty.
              std::swap(m_pushQueue, m_popQueue);
            }
          }
        }
        catch (...)
        {
        }
      }

      void ProcessLogs(LogItemQueue& logs)
      {
        while (!logs.empty())
        {
          ProcessLog(logs.front());
          logs.pop();
        }
      }

      void ProcessLog(const AsyncLogItem& logItem)
      {
        switch (logItem.type)
        {
          case AsyncLogItem::Type::COMMAND:
            ProcessLogCommand(logItem);
            break;

          case AsyncLogItem::Type::MESSAGE:
            ProcessLogMessage(logItem);
            break;

          default:
            assert(!"Unexpected log item type.");
            break;
        }
      }

      void ProcessLogCommand(const AsyncLogItem& logItem)
      {
        switch (logItem.command)
        {
          case AsyncLogItem::Command::PUSH_INDENT:
            ++m_threadIndents[logItem.threadId];
            break;

          case AsyncLogItem::Command::POP_INDENT:
          {
            unsigned& indent = m_threadIndents[logItem.threadId];
            if (indent > 0)
            {
              --indent;
            }
            break;
          }

          default:
            assert(!"Unexpected log item command.");
            break;
        }
      }

      void ProcessLogMessage(const LogItem& logItem)
      {
        AddLogItem(*m_logger, logItem, m_threadIndents[logItem.threadId]);
      }

      ThreadsafeAsyncLogger(const ThreadsafeAsyncLogger&) = delete;
      ThreadsafeAsyncLogger& operator = (const ThreadsafeAsyncLogger&) = delete;
    };


    class ThreadsafeSyncLogger : public Logger
    {
      using ThreadIndentMap = std::map<std::thread::id, unsigned>;

      std::unique_ptr<Logger> m_logger;
      ThreadIndentMap m_threadIndents;
      std::mutex m_mutex;

    public:
      explicit ThreadsafeSyncLogger(std::unique_ptr<Logger> logger)
        : m_logger(std::move(logger))
      {
        ERR_THROW_IF(m_logger.get() == nullptr, "Null logger.");
      }

      virtual void Add(const char* log, Level level) override
      {
        std::lock_guard<std::mutex> guard(m_mutex);
        LogItem logItem(log, level);
        const auto indent = m_threadIndents[logItem.threadId];
        AddLogItem(*m_logger, std::move(logItem), indent);
      }

      virtual void Add(const char* log, Level level, const char* meta, unsigned) override
      {
        std::lock_guard<std::mutex> guard(m_mutex);
        LogItem logItem(meta, log, level);
        const auto indent = m_threadIndents[logItem.threadId];
        AddLogItem(*m_logger, std::move(logItem), indent);
      }

      virtual void PushIndent() override
      {
        std::lock_guard<std::mutex> guard(m_mutex);
        ++m_threadIndents[std::this_thread::get_id()];
      }

      virtual void PopIndent() override
      {
        std::lock_guard<std::mutex> guard(m_mutex);
        auto& indent = m_threadIndents[std::this_thread::get_id()];
        if (indent > 0)
        {
          --indent;
        }
      }

      ThreadsafeSyncLogger(const ThreadsafeSyncLogger&) = delete;
      ThreadsafeSyncLogger& operator = (const ThreadsafeSyncLogger&) = delete;
    };
  }

  std::unique_ptr<Logger> CreateThreadsafeAsyncLogger(std::unique_ptr<Logger> logger)
  {
    return std::make_unique<ThreadsafeAsyncLogger>(std::move(logger));
  }

  std::unique_ptr<Logger> CreateThreadsafeSyncLogger(std::unique_ptr<Logger> logger)
  {
    return std::make_unique<ThreadsafeSyncLogger>(std::move(logger));
  }
}
}
