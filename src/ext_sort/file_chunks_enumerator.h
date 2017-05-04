#ifndef __EXT_SORT_FILE_LINES_ENUMERATOR_H__
#define __EXT_SORT_FILE_LINES_ENUMERATOR_H__

#include <ext_sort/types.h>

#include <memory>
#include <string>

namespace ExtSort
{
  namespace FileChunksEnumeratorEvents
  {
    const char* const BEFORE_READ_BUFFER = "FileChunksEnumeratorEvents::BeforeReadBuffer";
  }

  std::unique_ptr<CharsChunksEnumerator> CreateFileChunksEnumerator(
    const std::string& sourceFilePath,
    const CharsChunk& buffer,
    CharsChunk::ObjType chunksDelim);
}

#endif
