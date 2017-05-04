#ifndef __EXT_SORT_MERGER_H__
#define __EXT_SORT_MERGER_H__

#include <set>
#include <string>

namespace ExtSort
{
  class Merger
  {
  public:
    virtual ~Merger() = default;

    virtual void Merge(const std::set<std::string>& sortedFilePaths, const std::string& resultFilePath) = 0;
  };
}

#endif
