#ifndef __UTILS_FS_FS_H__
#define __UTILS_FS_FS_H__

#include <utils/enumerator.h>

#include <ios>
#include <cstdio>
#include <memory>
#include <string>

namespace Utils
{
  namespace Fs
  {
    typedef Enumerator<std::string> FilePathsEnumerator;

    struct FileDeleter
    {
      void operator () (FILE* file) const;
    };

    typedef std::unique_ptr<FILE, FileDeleter> FileUniquePtr;
    typedef std::streamsize Size;
    static_assert(sizeof(Size) >= 8, "Bad stream size type.");

    char GetPathSeparator();
    bool IsPathSeparator(char ch);
    std::string AppendPath(const std::string& parent, const std::string& child);
    bool IsExists(const std::string& path);
    Size GetSize(const std::string& path);
    FileUniquePtr OpenFile(const std::string& filePath, const char* mode);
    void EnsureDirExists(const std::string& path);
    void MoveFile(const std::string& source, const std::string& target);
    void RemoveFile(const std::string& filePath);
    void RemoveDir(const std::string& dirPath);
  }
}

#endif
