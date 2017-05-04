#include <utils/err.h>

#include <sstream>
#include <stdexcept>

namespace Utils
{
  namespace
  {
    std::ostream& EndLine(std::ostream& out)
    {
      out << "\r\n";
      return out;
    }
  }

  std::string FormatException(const std::exception& error)
  {
    std::ostringstream oss;
    oss << "std::exception:" << EndLine;
    oss << error.what() << EndLine;
    oss.flush();
    return oss.str();
  }

  std::string FormatException(std::exception_ptr error)
  {
    try
    {
      try
      {
        ERR_THROW_IF(error == nullptr, "Exception is null.");
        std::rethrow_exception(error);
      }
      catch (const std::exception& e)
      {
        return FormatException(e);
      }
      catch (...)
      {
        return std::string("Unknown exception");
      }
    }
    catch (...)
    {
      ERR_THROW("Awful exception during exception dispatching!");
    }

    return std::string();
  }

  std::string FormatErrorDescription(const std::string& descr, std::exception_ptr error)
  {
    std::ostringstream oss;
    oss << descr << EndLine;
    if (error != nullptr)
    {
      oss << "------------------------" << EndLine;
      oss << FormatException(error) << EndLine;
    }
    oss.flush();
    return oss.str();
  }

  std::string FormatErrorDescription(const std::string& descr, const char* file, const char* function, int line)
  {
    std::ostringstream oss;
    oss << "descr : " << descr << EndLine
        << "file  : " << (file ? file : "null file") << EndLine
        << "func  : " << (function ? function : "null function") << EndLine
        << "line  : " << line << EndLine;
    oss.flush();
    return oss.str();
  }

  void ThrowRuntimeException(const std::string& descr)
  {
    ThrowTypedException<std::runtime_error>(descr);
  }
}
