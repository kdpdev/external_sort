#include <run.h>
#include <predef.h>

#include <ext_sort/merge_sort_sorter.h>
#include <ext_sort/multi_files_per_phase_merger.h>

#include <utils/arg.h>
#include <utils/err.h>
#include <utils/fs/fs.h>
#include <utils/fs/simple_file_paths_enumerator.h>
#include <utils/log/log.h>
#include <utils/log/log_registry.h>
#include <utils/log/log_exception.h>
#include <utils/log/loggers/ostream_logger.h>
#include <utils/str_conv.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>

namespace
{
  const char* const ARG_USAGE_REQUEST       = "?";
  const char* const ARG_INPUT_FILE_PATH     = "input";
  const char* const ARG_OUTPUT_FILE_PATH    = "output";
  const char* const ARG_TEMP_DIR_PATH       = "temp_dir";
  const char* const ARG_MAX_MEMORY_USAGE_MB = "max_memory_usage_Mb";
  const char* const ARG_MAX_WRITE_BUFFER_KB = "max_write_buffer_Kb";
  const char* const ARG_REMOVE_TEMP_FILES   = "remove_temp_files";

  const char* const DEFAULT_MAX_MEMORY_USAGE_MB = "16";
  const char* const DEFAULT_MAX_WRITE_BUFFER_KB = "128";
  const char* const DEFAULT_REMOVE_TEMP_FILES   = "1";
  const char* const DEFAULT_TEMP_DIR_PATH       = "./temp/";

  class Usage
  {
    Utils::Arguments m_args;
    std::string m_appName;

  public:
    Usage(const Utils::Arguments& args)
      : m_args(args)
    {
      //m_args.SetDefault(ARG_INPUT_FILE_PATH     , "input");
      //m_args.SetDefault(ARG_OUTPUT_FILE_PATH    , "output");
      m_args.SetDefault(ARG_TEMP_DIR_PATH       , DEFAULT_TEMP_DIR_PATH);
      m_args.SetDefault(ARG_MAX_MEMORY_USAGE_MB , DEFAULT_MAX_MEMORY_USAGE_MB);
      m_args.SetDefault(ARG_MAX_WRITE_BUFFER_KB , DEFAULT_MAX_WRITE_BUFFER_KB);
      m_args.SetDefault(ARG_REMOVE_TEMP_FILES   , DEFAULT_REMOVE_TEMP_FILES);

      m_appName = m_args.GetArgument("app_path");
      auto lastSeparatorPos = m_appName.find_last_of(Utils::Fs::GetPathSeparator());
      #ifdef PREDEF_OS_WINDOWS
        if (lastSeparatorPos == std::string::npos)
        {
          lastSeparatorPos = m_appName.find_last_of('\\');
        }
      #endif
      if (lastSeparatorPos != std::string::npos)
      {
        m_appName = m_appName.substr(lastSeparatorPos + 1);
      }
    }

    bool HasUsageRequestArg() const
    {
      return m_args.HasArgument(ARG_USAGE_REQUEST);
    }

    void LogUsage() const
    {
      std::ostringstream oss;
      oss << std::endl;
      oss << m_appName << " usage:" << std::endl << std::endl;
      oss << "Parameters format is 'param_name=param_value'" << std::endl << std::endl;
      oss << m_appName
          << " " << ARG_INPUT_FILE_PATH
          << " " << ARG_OUTPUT_FILE_PATH
          << " [" << ARG_TEMP_DIR_PATH << "]"
          << " [" << ARG_MAX_MEMORY_USAGE_MB << "]"
          << " [" << ARG_MAX_WRITE_BUFFER_KB << "]"
          << " [" << ARG_REMOVE_TEMP_FILES << "]"
          << std::endl << std::endl;

      oss << "Where:" << std::endl;
      oss << "  " << ARG_INPUT_FILE_PATH      << " - file path to be sorted (must exists)." << std::endl;
      oss << "  " << ARG_OUTPUT_FILE_PATH     << " - result file path (must NOT exists)." << std::endl;
      oss << "  " << ARG_TEMP_DIR_PATH        << " - path to a directory for tempopary files (default value is '" + std::string(DEFAULT_TEMP_DIR_PATH) + "')." << std::endl;
      oss << "  " << ARG_MAX_MEMORY_USAGE_MB  << " - max memory usage in Mb (default value is '" + std::string(DEFAULT_MAX_MEMORY_USAGE_MB) + "')." << std::endl;
      oss << "  " << ARG_MAX_WRITE_BUFFER_KB  << " - max write buffer size in Kb (default value is '" + std::string(DEFAULT_MAX_WRITE_BUFFER_KB) + "')." << std::endl;
      oss << "  " << ARG_REMOVE_TEMP_FILES    << " - set to 1 to remove all temporary files (default value is '" + std::string(DEFAULT_REMOVE_TEMP_FILES) + "')." << std::endl;

      Utils::Log::GetLogger().Add(oss.str().c_str(), Utils::Log::LOG_LEVEL_INFO);
    }

    template <typename T>
    T GetArgument(const std::string& argName) const
    {
      return m_args.GetArgument<T>(argName);
    }

    void LogArgs() const
    {
      const auto& argsMap = m_args.GetAllArguments();
      const auto& maxArgName = std::max_element(argsMap.begin(), argsMap.end(), [] (const auto& lhs, const auto& rhs)
      {
        return lhs.first.length() < rhs.first.length();
      });

      std::ostringstream oss;
      oss << "Args:" << std::endl;
      for (const auto& entry : argsMap)
      {
        oss << "  " << entry.first << std::string(maxArgName->first.length() - entry.first.length(), ' ') << " : " << entry.second << std::endl;
      }

      LOG_I("\n%s", oss.str().c_str());
    }
  };

  struct LogHolder
  {
    ~LogHolder()
    {
      Utils::Log::SetLogger(Utils::Log::CreateCoutLogger());
    }
  };

  template <class Container>
  void LogSortedFiles(const Container& sortedFiles)
  {
    std::ostringstream oss;
    oss << "Sorted files (" << sortedFiles.size() << "):" << std::endl;
    for (const auto& file : sortedFiles)
    {
      oss << file << std::endl;
    }
    Utils::Log::GetLogger().Add(oss.str().c_str(), Utils::Log::LOG_LEVEL_INFO);
  }

  void Run(Utils::Arguments args)
  {
    Usage usage(args);

    if (usage.HasUsageRequestArg())
    {
      usage.LogUsage();
      return;
    }

    usage.LogArgs();

    std::string inputFilePath;
    std::string outputFilePath;
    std::string tempDirPath;
    std::size_t maxMemoryUsageMb;
    std::size_t maxWriteBufferKb;
    bool removeTempFiles = false;

    try
    {
      inputFilePath    = usage.GetArgument<std::string>(ARG_INPUT_FILE_PATH);
      outputFilePath   = usage.GetArgument<std::string>(ARG_OUTPUT_FILE_PATH);
      tempDirPath      = usage.GetArgument<std::string>(ARG_TEMP_DIR_PATH);
      maxMemoryUsageMb = usage.GetArgument<std::size_t>(ARG_MAX_MEMORY_USAGE_MB);
      maxWriteBufferKb = usage.GetArgument<std::size_t>(ARG_MAX_WRITE_BUFFER_KB);
      removeTempFiles  = usage.GetArgument<bool>(ARG_REMOVE_TEMP_FILES);
    }
    catch (...)
    {
      Utils::Log::LogCurrentException("Invalid arguments.");
      usage.LogUsage();
      return;
    }

    ERR_THROW_IF_NOT(Utils::Fs::IsExists(inputFilePath)   , "Input file not exists (path = '" + inputFilePath + "').");
    ERR_THROW_IF_NOT(!Utils::Fs::IsExists(outputFilePath) , "Output file already exists (path = '" + outputFilePath + "').");
    ERR_THROW_IF_NOT(maxMemoryUsageMb >= 1                , std::string(ARG_MAX_MEMORY_USAGE_MB) + " should be >= 1.");
    ERR_THROW_IF_NOT(maxWriteBufferKb >= 1                , std::string(ARG_MAX_WRITE_BUFFER_KB) + " should be >= 1.");

    const auto uniqueTempDirPath = Utils::Fs::AppendPath(tempDirPath, std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
    ERR_THROW_IF(Utils::Fs::IsExists(uniqueTempDirPath), "Temp dir already exists (path = '" + uniqueTempDirPath + "').");
    Utils::Fs::EnsureDirExists(uniqueTempDirPath);

    const auto maxMemoryUsageB = maxMemoryUsageMb << 20;
    const auto maxWriteBufferB = maxWriteBufferKb << 10;

    const auto mallocDeleter = [](void* ptr)
    {
      if (ptr)
      {
        free(ptr);
      }
    };
    std::unique_ptr<void, decltype(mallocDeleter)> bufferPtr(malloc(maxMemoryUsageB), mallocDeleter);
    ERR_THROW_IF_NOT(bufferPtr, "Failed to allocate " + std::to_string(maxMemoryUsageMb) + "Mb.");
    
    ExtSort::BytesChunk buffer;
    buffer.begin = (ExtSort::BytesChunk::ObjType*)bufferPtr.get();
    buffer.end = buffer.begin + maxMemoryUsageB / ExtSort::BytesChunk::SizeOfObject();

    const char linesDelim = '\n';

    std::set<std::string> sortedFiles;
    {
      LOG_SCOPE_I("SORT");
      auto filePathsEnumerator = Utils::Fs::CreateSimpleFilePathsEnumerator(uniqueTempDirPath, "sort", "");
      const auto sorter = ExtSort::CreateMergeSortSorter(
        std::move(filePathsEnumerator), buffer, maxWriteBufferB, linesDelim);
      sortedFiles = sorter->Sort(inputFilePath);
    }

    LogSortedFiles(sortedFiles);

    {
      LOG_SCOPE_I("MERGE");
      auto filePathsEnumerator = Utils::Fs::CreateSimpleFilePathsEnumerator(uniqueTempDirPath, "merge", "");
      auto merger = ExtSort::CreateMultiFilesPerPhaseMerger(
        std::move(filePathsEnumerator),
        buffer,
        sortedFiles.size(),
        maxWriteBufferB,
        linesDelim,
        removeTempFiles);
      merger->Merge(sortedFiles, outputFilePath);
    }


    if (removeTempFiles)
    {
      LOG_I("Remove temp dir: '%s'", uniqueTempDirPath.c_str());
      Utils::Fs::RemoveDir(uniqueTempDirPath);
    }

    LOG_I("DONE");
    LOG_I("RESULT: %s", outputFilePath.c_str());
  }
}


int Run(int argc, char** argv)
{
  int result = -1;
  LogHolder logHolder;
  try
  {
    Run(Utils::Arguments(argc, argv));
    result = 0;
  }
  catch (...)
  {
    Utils::Log::LogCurrentException("FAILED");
  }
  return result;
}
