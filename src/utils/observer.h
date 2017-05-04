#ifndef __UTILS_OBSERVER_H__
#define __UTILS_OBSERVER_H__

#include <functional>
#include <string>

namespace Utils
{
  using StringsObserver = std::function<void(const std::string& data)>;
}

#endif
