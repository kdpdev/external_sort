#include <predef.h>

#include <utils/fs/fs.h>
#include <utils/err.h>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace Utils
{
namespace Fs
{
  namespace
  {
    #ifdef PREDEF_OS_WINDOWS

      struct _stat64 Stat(const std::string& name, int& err)
      {
        ERR_THROW_IF(name.empty(), "Invalid argument (name is empty).");
        struct _stat64 s;
        if (_stat64(name.c_str(), &s))
        {
          err = errno;
        }
        return s;
      }

      struct _stat64 Stat(const std::string& name)
      {
        int err = 0;
        const auto s = Stat(name, err);
        ERR_THROW_IF(err != 0, "_stat64 failed (error = " + std::to_string(err) + ", name = '" + name + "').");
        return s;
      }

      std::string MakeWindowsPath(std::string path)
      {
        std::replace(std::begin(path), std::end(path), '/', '\\');
        return path;
      }

    #else

      struct stat64 Stat(const std::string& name, int& err)
      {
        ERR_THROW_IF(name.empty(), "Invalid argument (name is empty).");
        struct stat64 s;
        if (stat64(name.c_str(), &s))
        {
          err = errno;
        }
        return s;
      }

      struct stat64 Stat(const std::string& name)
      {
        int err = 0;
        const auto s = Stat(name, err);
        ERR_THROW_IF(err != 0, "stat64 failed (error = " + std::to_string(err) + ").");
        return s;
      }

    #endif

    std::string EnsureQuoted(const std::string& str)
    {
      const auto majorQuote = '"';
      const auto minorQuote = '\'';

      auto isQuote = [=](char ch)
      {
        return ch == majorQuote || ch == minorQuote;
      };

      if (str.empty())
      {
        return std::string(2, majorQuote);
      }

      if (isQuote(str.front()))
      {
        ERR_THROW_TYPED_IF(str.front() != str.back(), std::invalid_argument, "Bad string.");
        return str;
      }

      return majorQuote + str + majorQuote;
    }
  }

  void FileDeleter::operator () (FILE* file) const
  {
    if (file)
    {
      fclose(file);
    }
  }

  char GetPathSeparator()
  {
    return '/';
  }

  bool IsPathSeparator(char ch)
  {
    return GetPathSeparator() == ch;
  }

  std::string AppendPath(const std::string& parent, const std::string& child)
  {
    if (parent.empty() && child.empty())
    {
      return std::string();
    }

    const auto pathSeparator = GetPathSeparator();

    auto parentBegin = parent.begin();
    auto parentEnd = parent.end();
    if (parentBegin != parentEnd && parent.back() == pathSeparator)
    {
      --parentEnd;
    }

    auto childBegin = child.begin();
    auto childEnd = child.end();
    if (childBegin != childEnd && child.front() == pathSeparator)
    {
      ++childBegin;
    }

    std::string result(parentBegin, parentEnd);
    result.append(1, pathSeparator);
    result.append(childBegin, childEnd);
    return result;
  }

  bool IsExists(const std::string& path)
  {
    int err = 0;
    Stat(path, err);
    ERR_THROW_IF(err != 0 && err != ENOENT, "Stat failed (error = " + std::to_string(err) + ", path = '" + path + "').");
    return err == 0;
  }

  Size GetSize(const std::string& path)
  {
    const auto s = Stat(path);
    return s.st_size;
  }

  FileUniquePtr OpenFile(const std::string& filePath, const char* mode)
  {
    FileUniquePtr file = FileUniquePtr(fopen(filePath.c_str(), mode));
    ERR_THROW_IF(file.get() == nullptr, "Failed to open file (file path = '" + filePath + "').");
    return std::move(file);
  }

  void EnsureDirExists(const std::string& path)
  {
    // TODO: do not use system.
    ERR_THROW_IF(path.empty(), "Invalid argument (path is empty).");
    #ifdef PREDEF_OS_WINDOWS
      const std::string cmd = "mkdir " + EnsureQuoted(MakeWindowsPath(path));
    #else
      const std::string cmd = "mkdir -p " + EnsureQuoted(path);
    #endif
    const auto err = system(cmd.c_str());
    ERR_THROW_IF(err != 0, "EnsureDirExists failed (error = " + std::to_string(err) + ", path = '" + path + "').");
  }

  void MoveFile(const std::string& source, const std::string& target)
  {
    // TODO: do not use system.
    ERR_THROW_IF_NOT(IsExists(source), "Path not exists (path = '" + source + "').");
    ERR_THROW_IF(IsExists(target), "Path exists (path = '" + target + "').");
    #ifdef PREDEF_OS_WINDOWS
      const std::string cmd = "move " + EnsureQuoted(MakeWindowsPath(source)) + " " + EnsureQuoted(MakeWindowsPath(target));
    #else
      const std::string cmd = "mv " + EnsureQuoted(source) + " " + EnsureQuoted(target);
    #endif
    const auto err = system(cmd.c_str());
    ERR_THROW_IF(err != 0, "MoveFile failed (error = " + std::to_string(err) + ", source = '" + source + "', target = '" + target + "').");
  }

  void RemoveFile(const std::string& filePath)
  {
    // TODO: do not use system.
    ERR_THROW_IF_NOT(IsExists(filePath), "Path not exists (path = '" + filePath + "').");
    #ifdef PREDEF_OS_WINDOWS
      const std::string cmd = "del /q /f " + EnsureQuoted(MakeWindowsPath(filePath));
    #else
      const std::string cmd = "rm " + EnsureQuoted(filePath);
    #endif
    const auto err = system(cmd.c_str());
    ERR_THROW_IF(err != 0, "RemoveFile failed (error = " + std::to_string(err) + ", path = '" + filePath + "').");
  }

  void RemoveDir(const std::string& dirPath)
  {
    // TODO: do not use system.
    ERR_THROW_IF_NOT(IsExists(dirPath), "Path not exists (path = '" + dirPath + "').");
    #ifdef PREDEF_OS_WINDOWS
      const std::string cmd = "rmdir /q /s " + EnsureQuoted(MakeWindowsPath(dirPath));
    #else
      const std::string cmd = "rm -r " + EnsureQuoted(dirPath);
    #endif
    const auto err = system(cmd.c_str());
    ERR_THROW_IF(err != 0, "RemoveDir failed (error = " + std::to_string(err) + ", path = '" + dirPath + "').");
  }
}
}
