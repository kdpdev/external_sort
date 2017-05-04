#include <utils/fs/simple_file_paths_enumerator.h>

#include <chrono>
#include <mutex>

namespace Utils
{
namespace Fs
{
  namespace
  {
    class SimpleFilePathsEnumerator : public FilePathsEnumerator
    {
    public:
      using EventsObserver = FilePathsEnumerator::EventsObserver;

    private:
      const std::string m_filePathTemplate;
      const std::string m_fileNameSuffix;
      std::mutex m_mutex;
      std::size_t m_number;

    public:
      SimpleFilePathsEnumerator(const std::string& rootDirPath,
                                const std::string& fileNamePrefix,
                                const std::string& fileNameSuffix)
        : m_filePathTemplate(Fs::AppendPath(rootDirPath, fileNamePrefix))
        , m_fileNameSuffix(fileNameSuffix)
        , m_number(0)
      {
      }

      virtual void SetObserver(EventsObserver) override
      {
      }

      virtual bool Next(std::string& filePath) override
      {
        std::lock_guard<std::mutex> guard(m_mutex);

        filePath = m_filePathTemplate
            + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())
            + "_" + std::to_string(m_number)
            + m_fileNameSuffix;

        ++m_number;

        return true;
      }

      SimpleFilePathsEnumerator(const SimpleFilePathsEnumerator&) = delete;
      SimpleFilePathsEnumerator& operator = (const SimpleFilePathsEnumerator&) = delete;
    };
  }

  std::unique_ptr<FilePathsEnumerator> CreateSimpleFilePathsEnumerator(
    const std::string& rootDirPath,
    const std::string& fileNamePrefix,
    const std::string& fileNameSuffix)
  {
    return std::make_unique<SimpleFilePathsEnumerator>(
      rootDirPath, fileNamePrefix, fileNameSuffix);
  }
}
}
