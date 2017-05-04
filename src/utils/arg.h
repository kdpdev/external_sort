#ifndef __UTILS_ARG_H__
#define __UTILS_ARG_H__

#include <utils/str_conv.h>

#include <map>
#include <string>

namespace Utils
{
  class Arguments
  {
  public:
    using ArgsMap = std::map<std::string, std::string>;

  private:
    ArgsMap m_args;

  public:
    Arguments(int argc, const char* const* argv);
    ArgsMap GetAllArguments() const;
    bool HasArgument(const std::string& name) const;
    std::string GetArgument(const std::string& name) const;
    std::string GetArgument(const std::string& name, const std::string& defaultValue) const;
    void AddArgument(const std::string& name, const std::string& value);
    void SetDefault(const std::string& name, const std::string& value);

    void GetArgument(const std::string& name, bool& result) const;

    template <typename T>
    void GetArgument(const std::string& name, T& result) const
    {
      const std::string value = GetArgument(name);
      Utils::FromString<T>(value, result);
    }

    template <typename T>
    T GetArgument(const std::string& name) const
    {
      T result;
      GetArgument(name, result);
      return result;
    }
  };
}

#endif
