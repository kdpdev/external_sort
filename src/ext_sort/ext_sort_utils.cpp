#include <ext_sort/ext_sort_utils.h>

#include <utils/fs/fs.h>

namespace ExtSort
{
  void SaveToNewFile(const std::string& filePath,
                     const CharsChunk* chunksArr,
                     const std::size_t chunksArrSize,
                     bool writeEndChar,
                     void* writeBuffer,
                     const std::size_t writeBufferSize)
  {
    ERR_THROW_IF(chunksArr == nullptr, "Invalid argument. (chunksArr is null.");
    ERR_THROW_IF(Utils::Fs::IsExists(filePath), "Fs entry is already exists (path = '" + filePath + "'.)");

    Utils::Fs::FileUniquePtr file = Utils::Fs::OpenFile(filePath, "wb");
    FILE* rawFile = file.get();

    if (writeBuffer && writeBufferSize)
    {
      const auto disableBufferResult = setvbuf(rawFile, (char*) writeBuffer, _IOFBF, writeBufferSize);
      ERR_THROW_IF(disableBufferResult != 0, "Failed to disable buffering (error = " + std::to_string(disableBufferResult) + ").");
    }

    const std::size_t tieSize = writeEndChar ? CharsChunk::SizeOfObject() : 0;
    for (auto it = chunksArr, end = chunksArr + chunksArrSize; it != end; ++it)
    {
      const auto chunk = *it;
      const auto writeRes = fwrite(chunk.begin, chunk.BytesCount() + tieSize, 1, rawFile);
      if (writeRes != 1)
      {
        const auto err = ferror(rawFile);
        ERR_THROW("Failed to write chunk to file (error = " + std::to_string(err) + ").");
      }
    }

    if (fflush(rawFile) != 0)
    {
      const int err = ferror(rawFile);
      ERR_THROW("Failed to flush file (err = " + std::to_string(err) + ").");
    }
  }

  std::string FormatDataSize(std::size_t size)
  {
    if (size < 1024)
    {
      return "~" + std::to_string(size) + " bytes";
    }
    if (size < 1024 << 10)
    {
      return "~" + std::to_string(size >> 10) + "Kb";
    }
    return "~" + std::to_string(size >> 20) + "Mb";
  }

  std::string FormatDataCount(std::size_t size)
  {
    if (size < 1000)
    {
      return "~" + std::to_string(size);
    }
    if (size < 1000000)
    {
      return "~" + std::to_string(size / 1000) + "K";
    }
    return "~" + std::to_string(size / 1000000) + "M";
  }

  std::string FormatDuration(std::chrono::system_clock::duration duration)
  {
    const auto totalMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    if (totalMilliseconds <= 5 * 1000)
    {
      return std::to_string(totalMilliseconds) + " msec";
    }
    return std::to_string(totalMilliseconds / 1000) + " sec";
  }
}
