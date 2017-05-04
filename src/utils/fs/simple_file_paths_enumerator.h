#ifndef __UTILS_FS_SIMPLE_FILE_PATHS_ENUMERATOR_H__
#define __UTILS_FS_SIMPLE_FILE_PATHS_ENUMERATOR_H__

#include <utils/fs/fs.h>

#include <memory>
#include <string>

namespace Utils
{
  namespace Fs
  {
    std::unique_ptr<FilePathsEnumerator> CreateSimpleFilePathsEnumerator(
      const std::string& rootDirPath,
      const std::string& fileNamePrefix,
      const std::string& fileNameSuffix);
  }
}

#endif
