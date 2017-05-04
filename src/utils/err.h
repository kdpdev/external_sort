#ifndef __UTILS_ERR_H__
#define __UTILS_ERR_H__

#include <exception>
#include <string>

namespace Utils
{
  std::string FormatException(const std::exception& e);
  std::string FormatException(std::exception_ptr e);

  std::string FormatErrorDescription(const std::string& descr, std::exception_ptr e);
  std::string FormatErrorDescription(const std::string& descr, const char* file, const char* function, int line);
  
  void ThrowRuntimeException(const std::string& descr);

  template <typename ExceptionType>
  void ThrowTypedException(const std::string& descr)
  {
    throw ExceptionType(descr);
  }
}


#define MAKE_THROW_CONTEXT(descr) ::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__)


#define ERR_THROW(descr)                                     ::Utils::ThrowRuntimeException(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_IF(condition, descr)     if (condition)    ::Utils::ThrowRuntimeException(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_IF_NOT(condition, descr) if (!(condition)) ::Utils::ThrowRuntimeException(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))


#define ERR_THROW_TYPED(type, descr)                                     ::Utils::ThrowTypedException<type>(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_TYPED_IF(condition, type, descr)     if (condition)    ::Utils::ThrowTypedException<type>(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_TYPED_IF_NOT(condition, type, descr) if (!(condition)) ::Utils::ThrowTypedException<type>(::Utils::FormatErrorDescription(descr, __FILE__, __FUNCTION__, __LINE__))


#define ERR_THROW_CONSTRUCTED(constructor)                                     throw (constructor(__FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_CONSTRUCTED_IF(condition, constructor)     if (condition)    throw (constructor(__FILE__, __FUNCTION__, __LINE__))
#define ERR_THROW_CONSTRUCTED_IF_NOT(condition, constructor) if (!(condition)) throw (constructor(__FILE__, __FUNCTION__, __LINE__))


#define ERR_THROW_OBJECT(type, obj)                               throw obj
#define ERR_THROW_OBJECT_IF(condition, obj)     if (condition)    throw obj
#define ERR_THROW_OBJECT_IF_NOT(condition, obj) if (!(condition)) throw obj

#endif
