#include <ext_sort/merge_sort_sorter.h>
#include <ext_sort/ext_sort_utils.h>
#include <ext_sort/file_chunks_enumerator.h>

#include <utils/align.h>
#include <utils/log/log.h>
#include <utils/err.h>
#include <utils/merge_sort.h>
#include <utils/scope_time_logger.h>

#include <algorithm>
#include <chrono>
#include <sstream>

namespace ExtSort
{
  namespace
  {
    class MergeSortSorter : public Sorter
    {
      using ChunksChunk = Chunk<CharsChunk>;

      std::unique_ptr<Utils::Fs::FilePathsEnumerator> m_filePaths;
      const CharsChunk::ObjType m_chunksDelim;
      BytesChunk m_writeBuffer;
      CharsChunk m_readBuffer;
      ChunksChunk m_chunksBuffer;

    public:
      MergeSortSorter(std::unique_ptr<Utils::Fs::FilePathsEnumerator> filePaths,
                      const BytesChunk& buffer,
                      std::size_t maxWriteBufferSize,
                      CharsChunk::ObjType chunksDelim)
        : m_filePaths(std::move(filePaths))
        , m_chunksDelim(chunksDelim)
      {
        ERR_THROW_IF_NOT(m_filePaths, "Invalid argument (file paths is null).");

        CheckChunk(buffer);

        LOG_I("MergeSortSorter::MergeSortSorter: buffer size = %s", FormatDataSize(buffer.ObjectsCount()).c_str());

        const auto byfferSize = buffer.BytesCount();
        const auto maxAcceptableWriteBufferSize = byfferSize / 10;
        const auto writeBufferSize = (std::min)(maxAcceptableWriteBufferSize, maxWriteBufferSize);

        m_writeBuffer.begin = (BytesChunk::ObjType*)(buffer.begin);
        m_writeBuffer.end = (BytesChunk::ObjType*)(buffer.begin + writeBufferSize);
        CheckChunk(m_writeBuffer, buffer.end);

        m_chunksBuffer.begin = Utils::GetAligned((ChunksChunk::ObjType*)(m_writeBuffer.end));
        m_chunksBuffer.end = Utils::GetAligned((ChunksChunk::ObjType*)buffer.end);
        AdjustEnd(m_chunksBuffer, buffer.end);

        const auto availableBufferedChunks = m_chunksBuffer.ObjectsCount() / 2 - m_chunksBuffer.ObjectsCount() % 2;
        m_chunksBuffer.end = m_chunksBuffer.begin + availableBufferedChunks;
        CheckChunk(m_chunksBuffer, buffer.end);

        m_readBuffer.begin = Utils::GetAligned((CharsChunk::ObjType*)m_chunksBuffer.end);
        m_readBuffer.end = Utils::GetAligned((CharsChunk::ObjType*)buffer.end);
        AdjustEnd(m_readBuffer, buffer.end);
      }

      virtual std::set<std::string> Sort(const std::string& sourceFilePath)
      {
        Utils::Log::ScopedInfoLog sortScope("MergeSortSorter::Sort");

        const auto startTime = std::chrono::system_clock::now();

        LOG_I("source file path    = '%s'", sourceFilePath.c_str());

        const auto sourceFileSize = Utils::Fs::GetSize(sourceFilePath);
        if (sourceFileSize == 0)
        {
          LOG_W("%s", ("The '" + sourceFilePath + "' file is empty.").c_str());
          std::string resultFilePath;
          ERR_THROW_IF_NOT(m_filePaths->Next(resultFilePath), "Cannot get next file path.");
          Utils::Fs::OpenFile(resultFilePath, "wb");
          std::set<std::string> files;
          files.insert(resultFilePath);
          return files;
        }

        Utils::Fs::Size sortedDataSize = 0;
        std::string sortedDataProgress;

        LOG_I("source file size    = %s", FormatDataSize(static_cast<std::size_t>(sourceFileSize)).c_str());

        const auto allChunks = m_chunksBuffer.ObjectsCount() / 2;
        auto freeChunks = allChunks;
        CharsChunk* chunksCursor = m_chunksBuffer.begin;
        CharsChunk* sortBuffer = m_chunksBuffer.begin + freeChunks;

        LOG_I("max chunks per file = %s", FormatDataCount(freeChunks).c_str());
        LOG_I("max chunk length    = %s", FormatDataSize(m_readBuffer.ObjectsCount()).c_str());

        std::set<std::string> resultFilePaths;

        const auto flushData = [&, this] ()
        {
          if (m_chunksBuffer.begin != chunksCursor)
          {
            std::string targetFilePath;
            ERR_THROW_IF_NOT(m_filePaths->Next(targetFilePath), "Cannot get next file path.");

            const auto chunksArr = m_chunksBuffer.begin;
            const auto chunksArrSize = std::distance(m_chunksBuffer.begin, chunksCursor);
            const auto chunksDataSize = std::distance(chunksArr[0].begin, chunksArr[chunksArrSize - 1].end) + 1;
            SortAndSave(sortBuffer, chunksArr, chunksArrSize, targetFilePath);
            resultFilePaths.insert(targetFilePath);
            chunksCursor = m_chunksBuffer.begin;
            freeChunks = allChunks;

            sortedDataSize += chunksDataSize;
            std::string progress = FormatPart(sourceFileSize, sortedDataSize);
            if (progress != sortedDataProgress)
            {
              sortedDataProgress.swap(progress);
              LOG_I("Sort progress: %s", sortedDataProgress.c_str());
            }
          }
        };

        const auto enumeratorObserver = [&, this] (const std::string& eventId)
        {
          if (eventId == FileChunksEnumeratorEvents::BEFORE_READ_BUFFER)
          {
            flushData();
          }
        };

        auto enumerator = CreateFileChunksEnumerator(sourceFilePath, m_readBuffer, m_chunksDelim);
        enumerator->SetObserver(enumeratorObserver);

        CharsChunk chunk;
        while (enumerator->Next(chunk))
        {
          *chunksCursor = chunk;
          ++chunksCursor;
          --freeChunks;
          if (freeChunks == 0)
          {
            flushData();
          }
        }

        flushData();

        LOG_I("DONE: 100%%");
        LOG_I("Sort file time = %s", FormatDuration(std::chrono::system_clock::now() - startTime).c_str());

        return std::move(resultFilePaths);
      }

    private:
      void SortAndSave(CharsChunk* buf, CharsChunk* arr, std::size_t size, const std::string& outputFilePath)
      {
        Utils::Log::ScopedInfoLog sortScope("MergeSortSorter::SortAndSave");

        ERR_THROW_IF(arr == nullptr, "Invalid argument (array = null).");
        ERR_THROW_IF(size == 0, "Invalid argument (size = 0 null).");

        LOG_I("output file path   = '%s'" , outputFilePath.c_str());
        LOG_I("chunks count       = %s"   , FormatDataCount(size).c_str());
        LOG_I("total chunks size  = %s"   , FormatDataSize(std::distance(arr[0].begin, arr[size - 1].end) + 1).c_str());
        
        const auto startTime = std::chrono::system_clock::now();

        Utils::MergeSort(buf, arr, 0, size - 1);
        const auto sortDuration = (std::chrono::system_clock::now() - startTime).count();

        SaveToNewFile(outputFilePath, arr, size, true, m_writeBuffer.begin, m_writeBuffer.BytesCount());
        const auto saveDuration = (std::chrono::system_clock::now() - startTime).count() - sortDuration;

        const auto totalDuration = std::chrono::system_clock::now() - startTime;

        if (totalDuration.count() > 0)
        {
          std::ostringstream oss;
          oss << "total time         = " << FormatDuration(totalDuration);
          oss << " : sort " << FormatPart(totalDuration.count(), sortDuration);
          oss << " : save " << FormatPart(totalDuration.count(), saveDuration);
          LOG_I("%s", oss.str().c_str());
        }
      }

      MergeSortSorter(const MergeSortSorter&) = delete;
      MergeSortSorter& operator = (const MergeSortSorter&) = delete;
    };
  }

  std::unique_ptr<Sorter> CreateMergeSortSorter(
    std::unique_ptr<Utils::Fs::FilePathsEnumerator> filePaths,
    const BytesChunk& buffer,
    const std::size_t maxWriteBufferSize,
    CharsChunk::ObjType chunksDelim)
  {
    return std::make_unique<MergeSortSorter>(
      std::move(filePaths), buffer, maxWriteBufferSize, chunksDelim);
  }
}
