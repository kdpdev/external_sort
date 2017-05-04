#include <ext_sort/file_chunks_enumerator.h>
#include <ext_sort/ext_sort_utils.h>

#include <utils/empty_enumerator.h>
#include <utils/err.h>
#include <utils/fs/fs.h>

#include <cstring>

namespace ExtSort
{
  namespace
  {
    class ChunksEnumerator : public CharsChunksEnumerator
    {
      using Char = CharsChunk::ObjType;
      using EventsObserver = CharsChunksEnumerator::EventsObserver;

      const Char m_chunksDelim;
      const std::size_t m_bufferCapacity;
      Char* const m_bufferDataPtr;
      Char* m_cursor;
      Char* m_end;
      std::size_t m_lastChunkOffset;
      EventsObserver m_observer;
      Utils::Fs::FileUniquePtr m_file;

    public:
      ChunksEnumerator(const std::string& sourceFilePath,
                       const CharsChunk& buffer,
                       CharsChunk::ObjType chunksDelim)
        : m_chunksDelim(chunksDelim)
        , m_bufferCapacity(buffer.ObjectsCount())
        , m_bufferDataPtr(buffer.begin)
        , m_cursor(nullptr)
        , m_end(nullptr)
        , m_lastChunkOffset(0)
      {
        CheckChunk(buffer);

        const auto fileSize = Utils::Fs::GetSize(sourceFilePath);
        ERR_THROW_IF(fileSize == 0, "File is empty (path = '" + sourceFilePath + "').");
        ERR_THROW_IF(fileSize % CharsChunk::SizeOfObject() != 0, "fileSize % CharsChunk::SizeOfObject() != 0 (fileSize = " + std::to_string(fileSize) + ", CharsChunk::SizeOfObject() = " + std::to_string(CharsChunk::SizeOfObject()) + ", path = '" + sourceFilePath + "').");
        ERR_THROW_IF(m_bufferCapacity < CharsChunk::SizeOfObject(), "Invalid argument (buffer capacity is too small).");
        ERR_THROW_IF(m_bufferDataPtr == nullptr, "Invalid argument (buffer data is null).");

        m_file = Utils::Fs::OpenFile(sourceFilePath, "rb");
        const auto disableBufferResult = setvbuf(m_file.get(), nullptr, _IONBF, 0);
        ERR_THROW_IF(disableBufferResult != 0, "Failed to disable buffering (error = " + std::to_string(disableBufferResult) + ").");
      }

      virtual void SetObserver(EventsObserver observer) override
      {
        if (observer)
        {
          m_observer = observer;
        }
        else
        {
          m_observer = [] (const std::string&) {};
        }
      }

      virtual bool Next(CharsChunk& chunk) override
      {
        if (m_cursor != m_end)
        {
          auto* cursor = m_cursor;
          auto linesDelim = m_chunksDelim;
          for (; *cursor != linesDelim; ++cursor)
          {
          }
          chunk.begin = m_cursor;
          chunk.end = cursor;
          m_cursor = cursor + 1;
          return true;
        }

        FILE* file = m_file.get();

        if (feof(file))
        {
          return false;
        }

        if (m_observer)
        {
          m_observer(FileChunksEnumeratorEvents::BEFORE_READ_BUFFER);
        }

        auto* bufferData = m_bufferDataPtr;
        auto bufferSize = m_bufferCapacity;
        if (m_lastChunkOffset != 0)
        {
          std::memcpy(m_bufferDataPtr, m_cursor, m_lastChunkOffset);
          bufferData += m_lastChunkOffset;
          bufferSize -= m_lastChunkOffset;
        }

        const auto read = fread(bufferData, sizeof(*bufferData), bufferSize, file);

        if (read != bufferSize)
        {
          if (const auto ferr = ferror(file))
          {
            ERR_THROW("Failed to read file data (error = " + std::to_string(ferr) + ").");
          }

          if (!feof(file))
          {
            ERR_THROW("Unexpected file state. EOF is expected.");
          }

          if (read == 0)
          {
            return false;
          }

          if (bufferData[read - 1] != m_chunksDelim)
          {
            ERR_THROW("Unexpected end of file. Char with the '" + std::to_string(int(m_chunksDelim)) + "' code is expected.");
          }

          m_lastChunkOffset = 0;
          m_cursor = m_bufferDataPtr;
          m_end = bufferData + read;
        }
        else
        {
          const auto linesDelim = m_chunksDelim;
          auto* end = bufferData + read - 1;
          for (auto* rend = m_bufferDataPtr + m_lastChunkOffset; *end != linesDelim; --end)
          {
            if (end == rend)
            {
              ERR_THROW("Line length is exceeded max length (max length = " + std::to_string(m_bufferCapacity) + ").");
            }
          }

          m_cursor = m_bufferDataPtr;
          m_end = end + 1;
          m_lastChunkOffset = bufferData + read - m_end;
        }

        return ChunksEnumerator::Next(chunk);
      }

      ChunksEnumerator(const ChunksEnumerator&) = delete;
      ChunksEnumerator& operator = (const ChunksEnumerator&) = delete;
    };
  }

  std::unique_ptr<CharsChunksEnumerator> CreateFileChunksEnumerator(
    const std::string& sourceFilePath,
    const CharsChunk& buffer,
    CharsChunk::ObjType chunksDelim)
  {
    if (Utils::Fs::GetSize(sourceFilePath) == 0)
    {
      return std::make_unique<Utils::EmptyEnumerator<CharsChunk>>();
    }

    return std::make_unique<ChunksEnumerator>(sourceFilePath, buffer, chunksDelim);
  }
}
