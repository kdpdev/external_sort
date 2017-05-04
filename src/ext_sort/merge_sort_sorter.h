#ifndef __EXT_SORT_MERGE_SORT_SORTER_H__
#define __EXT_SORT_MERGE_SORT_SORTER_H__

#include <ext_sort/sorter.h>
#include <ext_sort/types.h>

#include <utils/fs/fs.h>

#include <memory>

namespace ExtSort
{
  std::unique_ptr<Sorter> CreateMergeSortSorter(
    std::unique_ptr<Utils::Fs::FilePathsEnumerator> filePaths,
    const BytesChunk& buffer,
    std::size_t maxWriteBufferSize,
    CharsChunk::ObjType chunksDelim);
}

#endif
