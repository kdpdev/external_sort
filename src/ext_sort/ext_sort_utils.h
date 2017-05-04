#ifndef __EXT_SORT_EXT_SORT_UTILS_H__
#define __EXT_SORT_EXT_SORT_UTILS_H__

#include <ext_sort/types.h>

#include <utils/align.h>
#include <utils/err.h>

#include <chrono>

namespace ExtSort
{
  void SaveToNewFile(const std::string& filePath,
                     const CharsChunk* chunksArr,
                     std::size_t chunksArrSize,
                     bool writeEndChar,
                     void* writeBuffer,
                     std::size_t writeBufferSize);

  template <typename T>
  void CheckAligned(const Chunk<T>& chunk)
  {
      Utils::CheckAligned(chunk.begin);
      Utils::CheckAligned(chunk.end);
      // Check if an array is without holes. Actual in case of some custom alignment of ObjType.
      Utils::CheckAligned(chunk.begin + 1);
  }

  template <typename T>
  void CheckChunk(const Chunk<T>& chunk)
  {
    CheckAligned(chunk);
    ERR_THROW_IF(chunk.begin == nullptr, "Bad chunk (chunk.begin points to null).");
    ERR_THROW_IF(chunk.end == nullptr, "Bad chunk (chunk.end points to null).");
    ERR_THROW_IF(chunk.begin - chunk.end > 0, "Bad chunk (invalid range).");
    ERR_THROW_IF(chunk.ObjectsCount() == 0, "Bad chunk (chunk is empty).");
  }

  template <typename T>
  void CheckChunk(const Chunk<T>& chunk, const void* endLimit)
  {
    CheckChunk(chunk);
    ERR_THROW_IF(endLimit == nullptr, "Invalid argument (endLimit is null.");
    ERR_THROW_IF((const Byte*)chunk.end - (const Byte*)endLimit > 0, "Bad chunk (limit is exceeded).");
  }

  template <typename T>
  void AdjustEnd(Chunk<T>& chunk, const void* endLimit)
  {
    CheckChunk(chunk);
    ERR_THROW_IF(endLimit == nullptr, "Invalid argument (endLimit is null.");
    if ((const Byte*)chunk.end - (const Byte*)endLimit > 0)
    {
      --chunk.end;
    }
    CheckChunk(chunk, endLimit);
  }

  std::string FormatDataSize(std::size_t size);
  std::string FormatDataCount(std::size_t size);
  std::string FormatDuration(std::chrono::system_clock::duration duration);

  template <typename T>
  std::string FormatPart(T total, T part)
  {
    ERR_THROW_IF(total < 0    , "Invalid arguments (total < 0).");
    ERR_THROW_IF(part  < 0    , "Invalid arguments (part < 0).");
    ERR_THROW_IF(total < part , "Invalid arguments (total < part).");
    return std::to_string(total == 0 ? 0 : static_cast<int>((double(part) / total) * 100)) + " %";
  }
}

#endif
