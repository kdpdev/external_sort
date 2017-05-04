#include <ext_sort/multi_files_per_phase_merger.h>

#include <ext_sort/ext_sort_utils.h>
#include <ext_sort/file_chunks_enumerator.h>

#include <utils/align.h>
#include <utils/err.h>
#include <utils/log/log.h>
#include <utils/merge_sort.h>

#include <algorithm>
#include <chrono>
#include <map>
#include <numeric>
#include <sstream>
#include <vector>

namespace ExtSort
{
  namespace
  {
    class MultiFilesPerPhaseMerger : public Merger
    {
      using ChunksChunk = Chunk<CharsChunk>;

      struct ReadParams
      {
        std::string filePath;
        CharsChunk readBuffer;
      };

      struct MergeTask
      {
        std::string name;
        std::string resultFilePath;
        BytesChunk writeBuffer;
        std::vector<ReadParams> readParams;
      };


      std::unique_ptr<Utils::Fs::FilePathsEnumerator> m_tempFilePaths;
      const BytesChunk m_buffer;
      const std::size_t m_maxFilesPerPhase;
      const std::size_t m_maxWriteBufferSize;
      const CharsChunk::ObjType m_chunksDelim;
      const bool m_removeTempFiles;

    public:
      MultiFilesPerPhaseMerger(std::unique_ptr<Utils::Fs::FilePathsEnumerator> tempFilePaths,
                               const BytesChunk& buffer,
                               std::size_t maxFilesPerPhase,
                               std::size_t maxWriteBufferSize,
                               CharsChunk::ObjType chunksDelim,
                               bool removeTempFiles)
        : m_tempFilePaths(std::move(tempFilePaths))
        , m_buffer(buffer)
        , m_maxFilesPerPhase(maxFilesPerPhase)
        , m_maxWriteBufferSize(maxWriteBufferSize)
        , m_chunksDelim(chunksDelim)
        , m_removeTempFiles(removeTempFiles)
      {
        ERR_THROW_IF_NOT(m_tempFilePaths, "Invalid argument (file paths is null).");
        CheckChunk(m_buffer);
      }

      virtual void Merge(const std::set<std::string>& sortedFilePaths, const std::string& resultFilePath) override
      {
        Utils::Log::ScopedInfoLog sortScope("MultiFilesPerPhaseMerger::Merge");

        const auto startTime = std::chrono::system_clock::now();

        LOG_I("result source files count = %s", std::to_string(sortedFilePaths.size()).c_str());
        LOG_I("result file path = '%s'", resultFilePath.c_str());

        ERR_THROW_IF(sortedFilePaths.empty(), "Invalid argument (sortedFilePaths path is empty).");
        ERR_THROW_IF(resultFilePath.empty(), "Invalid argument (resultFilePath is empty).");
        const auto emptySortedFilePathsCount = std::count_if(sortedFilePaths.begin(), sortedFilePaths.end(), [](const auto& path)
        {
          return path.empty();
        });
        ERR_THROW_IF(emptySortedFilePathsCount != 0, "Invalid argument (emptySortedFilePathsCount = " + std::to_string(emptySortedFilePathsCount) + ").");

        if (sortedFilePaths.size() == 1)
        {
          Utils::Fs::MoveFile(*sortedFilePaths.begin(), resultFilePath);
          return;
        }

        std::size_t completedMergeTasks = 0;
        const auto mergeTasks = GetMergeTasks(0, sortedFilePaths, resultFilePath);
        for (const auto& mergeTask : mergeTasks)
        {
          std::ostringstream scope;
          scope << "Merge task"
            << " : [" << (completedMergeTasks + 1) << "/ "<< mergeTasks.size() << "]"
            << " : ('" << mergeTask.name << "')"
            << " : files = " << mergeTask.readParams.size();
          Utils::Log::ScopedInfoLog mergeTaskScope(scope.str());
          const auto startMergeTime = std::chrono::system_clock::now();
          Merge(mergeTask);
          if (m_removeTempFiles)
          {
            for (const auto& readParams : mergeTask.readParams)
            {
              Utils::Fs::RemoveFile(readParams.filePath);
            }
          }
          ++completedMergeTasks;
          LOG_I("Done: time = %s", FormatDuration(std::chrono::system_clock::now() - startMergeTime).c_str());
        }

        LOG_I("DONE: 100 %%");
        LOG_I("Merge time = %s", FormatDuration(std::chrono::system_clock::now() - startTime).c_str());
      }

    private:
      static decltype(auto) CreateProgress(const MergeTask& mergeTask)
      {
        const auto totalFilesSize = std::accumulate(mergeTask.readParams.begin(), mergeTask.readParams.end(), std::size_t(0), [](auto summ, const auto& rp)
        {
          return summ + static_cast<std::size_t>(Utils::Fs::GetSize(rp.filePath));
        });

        return [totalFilesSize, processedBytes = std::size_t(0), percents = -1](std::size_t bytes, bool done) mutable
        {
          if (totalFilesSize)
          {
            processedBytes += bytes;
            const auto newPercents = done ? 100 : static_cast<int>(100.0 * processedBytes / totalFilesSize);
            if (newPercents != percents || percents < 0)
            {
              if (percents < 0 || newPercents / 10 != percents / 10)
              {
                percents = newPercents;
                LOG_I("merge progress: %d %%", percents);
              }
            }
          }
        };
      }

      void Merge(const MergeTask& mergeTask) const
      {
        ERR_THROW_IF(Utils::Fs::IsExists(mergeTask.resultFilePath), "Fs entry is already exists (path = '" + mergeTask.resultFilePath + "'.)");

        const auto resultFile = Utils::Fs::OpenFile(mergeTask.resultFilePath, "wb");
        FILE* rawResultFile = resultFile.get();
        if (const auto writeBufferSize = mergeTask.writeBuffer.BytesCount())
        {
          const auto disableBufferResult = setvbuf(rawResultFile, (char*) mergeTask.writeBuffer.begin, _IOFBF, writeBufferSize);
          ERR_THROW_IF(disableBufferResult != 0, "Failed to disable buffering (error = " + std::to_string(disableBufferResult) + ").");
        }

        constexpr auto writeTie = CharsChunk::SizeOfObject();
        const auto writeChunk = [rawResultFile, writeTie] (CharsChunk chunk)
        {
          const auto writeRes = fwrite(chunk.begin, chunk.BytesCount() + writeTie, 1, rawResultFile);
          if (writeRes != 1)
          {
            const auto err = ferror(rawResultFile);
            ERR_THROW("Failed to write chunk to file (error = " + std::to_string(err) + ").");
          }
        };


        using EnumeratorPtr = std::unique_ptr<CharsChunksEnumerator>;
        std::vector<EnumeratorPtr> enumerators;
        std::multimap<CharsChunk, CharsChunksEnumerator*> mergeItems;
        for (const auto& rp : mergeTask.readParams)
        {
          auto enumerator = CreateFileChunksEnumerator(rp.filePath, rp.readBuffer, m_chunksDelim);
          CharsChunk chunk;
          if (enumerator->Next(chunk))
          {
            auto enumeratorPtr = enumerator.get();
            enumerators.push_back(std::move(enumerator));
            mergeItems.emplace(chunk, enumeratorPtr);
          }
        }

        auto progress = CreateProgress(mergeTask);
        progress(0, false);

        while(!mergeItems.empty())
        {
          std::pair<CharsChunk, CharsChunksEnumerator*> tmp = *mergeItems.begin();

          writeChunk(tmp.first);
          progress(tmp.first.BytesCount(), false);

          mergeItems.erase(mergeItems.begin());
          if (tmp.second->Next(tmp.first))
          {
            mergeItems.insert(std::move(tmp));
          }
        }

        progress(0, true);

        if (fflush(rawResultFile) != 0)
        {
          const int err = ferror(rawResultFile);
          ERR_THROW("Failed to flush file (err = " + std::to_string(err) + ").");
        }
      }

      std::vector<MergeTask> GetMergeTasks(
        unsigned phase,
        const std::set<std::string>& sortedFilePaths,
        const std::string& resultFilePath) const
      {
        const auto sortedFilesCount = sortedFilePaths.size();
        ERR_THROW_IF(sortedFilesCount < 2, "Invalid argument (sortedFilesCount < 2).");
        ERR_THROW_IF(m_maxFilesPerPhase < 2, "Invalid state (maxFilesPerPhase < 2).");

        if (sortedFilesCount <= m_maxFilesPerPhase)
        {
          MergeTask mergeTask;
          mergeTask.name = std::to_string(phase) + "." + std::to_string(0);
          mergeTask.resultFilePath = resultFilePath;
          mergeTask.readParams.reserve(sortedFilesCount);
          for (const auto& filePath : sortedFilePaths)
          {
            ReadParams readParams;
            readParams.filePath = filePath;
            mergeTask.readParams.push_back(std::move(readParams));
          }
          SetupBuffers(mergeTask);
          return{ mergeTask };
        }

        std::vector<MergeTask> mergeTasks;
        std::set<std::string> thisPhaseFilePaths;
        auto sortedFilePathIter = sortedFilePaths.begin();
        const std::size_t tasksCount = sortedFilesCount / m_maxFilesPerPhase + ((sortedFilesCount % m_maxFilesPerPhase) ? 1 : 0);
        for (std::size_t i = 0; i != tasksCount; ++i)
        {
          MergeTask mergeTask;
          mergeTask.name = std::to_string(phase) + "." + std::to_string(i);
          ERR_THROW_IF_NOT(m_tempFilePaths->Next(mergeTask.resultFilePath), "Cannot get temp file path.");
          thisPhaseFilePaths.insert(mergeTask.resultFilePath);
          mergeTask.readParams.reserve(sortedFilesCount);
          for (std::size_t j = 0, end = sortedFilesCount / tasksCount; j != end; ++j)
          {
            ReadParams readParams;
            readParams.filePath = *sortedFilePathIter++;
            mergeTask.readParams.push_back(std::move(readParams));
          }

          if (const std::size_t odd = sortedFilesCount % tasksCount)
          {
            if (i < odd)
            {
              ReadParams readParams;
              readParams.filePath = *sortedFilePathIter++;
              mergeTask.readParams.push_back(std::move(readParams));
            }
          }

          SetupBuffers(mergeTask);
          mergeTasks.push_back(mergeTask);
        }

        const std::vector<MergeTask> nextPhaseTasks = GetMergeTasks(phase + 1, thisPhaseFilePaths, resultFilePath);
        mergeTasks.insert(mergeTasks.end(), nextPhaseTasks.begin(), nextPhaseTasks.end());
        return std::move(mergeTasks);
      }

      void SetupBuffers(MergeTask& mergeTask) const
      {
        const BytesChunk buffer = m_buffer;
        const auto byfferSize = buffer.BytesCount();

        const auto maxAcceptableWriteBufferSize = byfferSize / (mergeTask.readParams.size() + 1);
        const auto writeBufferSize = (std::min)(maxAcceptableWriteBufferSize, m_maxWriteBufferSize);
        mergeTask.writeBuffer.begin = (BytesChunk::ObjType*)(buffer.begin);
        mergeTask.writeBuffer.end = (BytesChunk::ObjType*)(buffer.begin + writeBufferSize);
        CheckChunk(mergeTask.writeBuffer, buffer.end);

        CharsChunk readBuffer;
        readBuffer.begin = Utils::GetAligned((CharsChunk::ObjType*)(mergeTask.writeBuffer.end));
        readBuffer.end = Utils::GetAligned((CharsChunk::ObjType*)buffer.end);
        AdjustEnd(readBuffer, buffer.end);
        const auto availableReadBufferSize = readBuffer.ObjectsCount();
        const auto requiredReadBuffers = mergeTask.readParams.size();
        ERR_THROW_IF(availableReadBufferSize < requiredReadBuffers, "Buffer is too small.");

        auto minReadBufferPerFile = availableReadBufferSize / requiredReadBuffers;
        std::size_t offset = 0;
        for (std::size_t i = 0; i != requiredReadBuffers; ++i)
        {
          auto bufferSize = minReadBufferPerFile;
          if (const auto odd = availableReadBufferSize % minReadBufferPerFile)
          {
            if (i < odd)
            {
              bufferSize += 1;
            }
          }
          CharsChunk& rb = mergeTask.readParams[i].readBuffer;
          rb.begin = readBuffer.begin + offset;
          rb.end = rb.begin + bufferSize;
          CheckChunk(rb, buffer.end);
          offset += bufferSize;
        }
      }

      MultiFilesPerPhaseMerger(const MultiFilesPerPhaseMerger&) = delete;
      MultiFilesPerPhaseMerger& operator = (const MultiFilesPerPhaseMerger&) = delete;
    };
  }

  std::unique_ptr<Merger> CreateMultiFilesPerPhaseMerger(
    std::unique_ptr<Utils::Fs::FilePathsEnumerator> tempFilePaths,
    const BytesChunk& buffer,
    std::size_t maxFilesPerPhase,
    std::size_t maxWriteBufferSize,
    CharsChunk::ObjType chunksDelim,
    bool removeTempFiles)
  {
    return std::make_unique<MultiFilesPerPhaseMerger>(
      std::move(tempFilePaths),
      buffer,
      maxFilesPerPhase,
      maxWriteBufferSize,
      chunksDelim,
      removeTempFiles);
  }
}
