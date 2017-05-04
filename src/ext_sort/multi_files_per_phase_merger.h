#ifndef __EXT_SORT_MULTI_FILES_MERGER_H__
#define __EXT_SORT_MULTI_FILES_MERGER_H__

#include <ext_sort/merger.h>
#include <ext_sort/types.h>

#include <utils/fs/fs.h>

#include <memory>

namespace ExtSort
{
  std::unique_ptr<Merger> CreateMultiFilesPerPhaseMerger(
    std::unique_ptr<Utils::Fs::FilePathsEnumerator> tempFilePaths,
    const BytesChunk& buffer,
    std::size_t maxFilesPerPhase,
    std::size_t maxWriteBufferSize,
    CharsChunk::ObjType chunksDelim,
    bool removeTempFiles);
}

#endif
