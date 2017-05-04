#ifndef __UTILS_STR_CONV_H__
#define __UTILS_STR_CONV_H__

#include <sstream>
#include <string>

namespace Utils
{
  template <typename T, typename Char>
  void FromString(const std::basic_string<Char>& str, T& result)
  {
    std::basic_istringstream<Char> iss(str);
    iss.exceptions(std::istream::failbit | std::istream::badbit);
    iss >> result;
  }

  template <typename Char>
  void FromString(const std::basic_string<Char>& str, std::basic_string<Char>& result)
  {
    result = str;
  }

  template <typename T, typename Char>
  T FromString(const std::basic_string<Char>& str)
  {
    T result;
    FromString(str, result);
    return result;
  }
}

#endif
