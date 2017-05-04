#include <utils/arg.h>
#include <utils/err.h>

#include <algorithm>
#include <stdexcept>

namespace Utils
{
  namespace
  {
    using Arg = Arguments::ArgsMap::value_type;

    std::string Trim(const std::string& str)
    {
      const auto notSpace = [] (char ch)
      {
        return ch < 0 || !isspace(ch);
      };
      const auto front = std::find_if(str.begin(), str.end(), notSpace);
      const auto back = std::find_if(str.rbegin(), str.rend(), notSpace);
      return std::string(front, back == str.rend() ? str.end() : back.base());
    }

    Arg StringToArg(const std::string& argLine)
    {
      if (argLine.empty())
      {
        return Arg();
      }

      auto line = argLine;
      if (argLine.size() >= 2 && argLine.front() == '\"' && argLine.back() == '\"')
      {
        line = argLine.substr(1, argLine.size() - 2);
      }

      const auto delimPos = line.find('=');

      if (delimPos == std::string::npos)
      {
        return Arg(Trim(line), std::string());
      }

      if (delimPos == line.length())
      {
        return Arg(Trim(std::string(line.begin(), line.begin() + delimPos)),
                   std::string());
      }

      return Arg(Trim(std::string(line.begin(), line.begin() + delimPos)),
                 Trim(std::string(line.begin() + delimPos + 1, line.end())));
    }
  }

  Arguments::Arguments(int argc, const char* const* argv)
  {
    if (argc > 0)
    {
      AddArgument("app_path", argv[0]);
    }

    for (auto i = 1; i < argc; ++i)
    {
      const Arg& arg = StringToArg(argv[i]);
      AddArgument(arg.first, arg.second);
    }
  }

  Arguments::ArgsMap Arguments::GetAllArguments() const
  {
    return m_args;
  }

  bool Arguments::HasArgument(const std::string& name) const
  {
    return m_args.find(name) != m_args.end();
  }

  std::string Arguments::GetArgument(const std::string& name) const
  {
    const auto iter = m_args.find(name);
    if (iter == m_args.end())
    {
      ERR_THROW_TYPED(std::invalid_argument, "Argument not found (name = '" + name + "').");
    }
    return iter->second;
  }

  std::string Arguments::GetArgument(const std::string& name, const std::string& defaultValue) const
  {
    return HasArgument(name)
      ? GetArgument(name)
      : defaultValue;
  }

  void Arguments::GetArgument(const std::string& name, bool& result) const
  {
    const auto value = GetArgument(name);
    if (value == "true" || value == "1" || value == "yes")
    {
      result = true;
      return;
    }
    if (value == "false" || value == "0" || value == "no")
    {
      result = false;
      return;
    }
    ERR_THROW_TYPED(std::logic_error, "Unexpected bool argument value (name = '" + name + "', value = '" + value + "').");
  }

  void Arguments::AddArgument(const std::string& name, const std::string& value)
  {
    m_args.emplace(name, value);
  }

  void Arguments::SetDefault(const std::string& name, const std::string& value)
  {
    if (!HasArgument(name))
    {
      AddArgument(name, value);
    }
  }
}
