#ifndef __EXT_SORT_TYPES_H__
#define __EXT_SORT_TYPES_H__

#include <utils/enumerator.h>

#include <string>

namespace ExtSort
{
  template <typename T>
  struct Chunk
  {
    using ObjType = T;

    ObjType* begin;
    ObjType* end;

    Chunk()
      : begin(nullptr)
      , end(nullptr)
    {
    }

    Chunk(ObjType* theBegin, ObjType* theEnd)
      : begin(theBegin)
      , end(theEnd)
    {
    }

    std::size_t ObjectsCount() const
    {
      return end - begin;
    }

    std::size_t BytesCount() const
    {
      return ObjectsCount() * SizeOfObject();
    }

    static constexpr std::size_t SizeOfObject()
    {
      return sizeof(ObjType);
    }
  };

  template <typename T>
  bool operator < (const Chunk<T>& lhs, const Chunk<T>& rhs)
  {
    const auto lhsCount = lhs.ObjectsCount();
    const auto rhsCount = rhs.ObjectsCount();

    /////////////////////////////////////////////////////////////////////////////
    // (lhsCount < rhsCount) + (cmpRes == 0) + [(lhsCount < rhsCount) or (cmpRes < 0)]
    // vs
    // (lhsCount < rhsCount) + [(cmpRes <= 0) or (cmpRes < 0)]
    /////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    //const auto cmpRes = std::char_traits<T>::compare(lhs.begin, rhs.begin, lhsCount < rhsCount ? lhsCount : rhsCount);
    //return cmpRes == 0 ? (lhsCount < rhsCount) : (cmpRes < 0);
    /////////////////////////////////////////////////////////////////////////////
    if (lhsCount < rhsCount)
    {
      const auto cmpRes = std::char_traits<T>::compare(lhs.begin, rhs.begin, lhsCount);
      return cmpRes <= 0;
    }
    else
    {
      const auto cmpRes = std::char_traits<T>::compare(lhs.begin, rhs.begin, rhsCount);
      return cmpRes < 0;
    }
    /////////////////////////////////////////////////////////////////////////////
  }

  using Byte = char;
  using BytesChunk = Chunk<Byte>;
  using CharsChunk = Chunk<char>;
  using CharsChunksEnumerator = Utils::Enumerator<CharsChunk>;

  static_assert(sizeof(CharsChunk::ObjType) == 1 || sizeof(CharsChunk::ObjType) % 2 == 0, "Objects align issue.");
}

#endif
