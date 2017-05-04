#ifndef __EXT_SORT_SORTER_H__
#define __EXT_SORT_SORTER_H__

#include <set>
#include <string>

namespace ExtSort
{
  class Sorter
  {
  public:
    virtual ~Sorter() = default;

    virtual std::set<std::string> Sort(const std::string& sourceFilePath) = 0;
  };
}

#endif
