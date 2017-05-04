#include <utils/log/loggers/ostream_logger.h>
#include <utils/err.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace Utils
{
namespace Log
{
  namespace
  {
    const char* GetPrefixForLevel(Level level)
    {
      switch (level)
      {
        case LOG_LEVEL_ERROR   : return "e : ";
        case LOG_LEVEL_WARNING : return "w : ";
        case LOG_LEVEL_INFO    : return "i : ";
        case LOG_LEVEL_DEBUG   : return "d : ";
      }

      assert(!"Unexpected log level.");
      return "? : ";
    }

    const char* GetIndentForIndent(unsigned indent)
    {
      static const char fills[] = "                                                                                                                                ";
      const unsigned fillCount = (sizeof(fills) / sizeof(*fills)) - 1;
      const unsigned indentSize = 4;
      const unsigned offset = fillCount - (std::min)(indentSize * indent, fillCount);
      return fills + offset;
    }

    class OutStreamDeleter
    {
      bool m_destroy;

    public:
      explicit OutStreamDeleter(bool destroy)
        : m_destroy(destroy)
      {
      }

      void operator () (std::ostream* stream)
      {
        if (m_destroy)
        {
          delete stream;
        }
      }
    };

    using OutStreamPtr = std::unique_ptr<std::ostream, OutStreamDeleter>;

    class OutStreamLogger : public Logger
    {
      OutStreamPtr m_out;
      unsigned m_indent;

    public:
      explicit OutStreamLogger(OutStreamPtr out)
        : m_out(std::move(out))
        , m_indent(0)
      {
        ERR_THROW_IF(m_out.get() == nullptr, "Null output stream.");
      }

      virtual void Add(const char* log, Level level) override
      {
        Add(log, level, nullptr, m_indent);
      }

      virtual void Add(const char* log, Level level, const char* meta, unsigned indent) override
      {
        assert(log && "Null log message.");
        std::ostream& out = *m_out;
        out << GetPrefixForLevel(level)
            << (meta ? meta : "")
            << GetIndentForIndent(indent)
            << (log ? log : "Null log message.")
            << std::endl;
      }

      virtual void PushIndent() override
      {
        ++m_indent;
      }

      virtual void PopIndent() override
      {
        if (m_indent > 0)
        {
          --m_indent;
        }
      }

      OutStreamLogger(const OutStreamLogger&) = delete;
      OutStreamLogger& operator = (const OutStreamLogger&) = delete;
    };
  }

  std::unique_ptr<Logger> CreateOutStreamLogger(std::ostream& out)
  {
    return std::make_unique<OutStreamLogger>(OutStreamPtr(&out, OutStreamDeleter(false)));
  }

  std::unique_ptr<Logger> CreateOutStreamLogger(std::unique_ptr<std::ostream> out)
  {
    return std::make_unique<OutStreamLogger>(OutStreamPtr(out.release(), OutStreamDeleter(true)));
  }

  std::unique_ptr<Logger> CreateCoutLogger()
  {
    return CreateOutStreamLogger(std::cout);
  }

  std::unique_ptr<Logger> CreateFoutLogger(const std::string& filePath, bool truncateFile)
  {
    const auto flags = std::ios::out | (truncateFile ? std::ios::trunc : std::ios::app);
    auto fileStream = std::make_unique<std::ofstream>(filePath.c_str(), flags);
    ERR_THROW_IF_NOT(*fileStream, "Failed to open '" + filePath + "' file.");
    return CreateOutStreamLogger(std::move(fileStream));
  }
}
}
