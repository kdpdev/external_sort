#ifndef __UTILS_ALIGN_H__
#define __UTILS_ALIGN_H__

#include <predef.h>

#include <utils/err.h>

#include <memory>
#include <type_traits>

namespace Utils
{
  namespace Private
  {
    inline void* Align(std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space)
    {
      #ifdef USE_CUSTOM_ALIGN
        // Discussion  : https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57350
        // Code source : https://gcc.gnu.org/viewcvs/gcc/trunk/libstdc%2B%2B-v3/include/std/memory?view=markup&pathrev=216149
        const auto intptr = reinterpret_cast<uintptr_t>(ptr);
        const auto aligned = (intptr - 1u + alignment) & -alignment;
        const auto diff = aligned - intptr;
        if ((size + diff) > space)
        {
          return nullptr;
        }
        space -= diff;
        return ptr = reinterpret_cast<void*>(aligned);
      #else
        return std::align(alignment, size, ptr, space);
      #endif
    }
  }

  template <typename T>
  bool IsAligned(const T* objPtr)
  {
    if (!objPtr)
    {
      // null pointer is alligned.
      return true;
    }

    void* bufPtr = const_cast<T*>(objPtr);
    std::size_t bufSize = sizeof(T);
    auto aligned = Private::Align(std::alignment_of<T>::value, sizeof(T), bufPtr, bufSize);
    return objPtr == aligned;
  }

  template <typename T>
  T* CheckAligned(T* objPtr)
  {
    ERR_THROW_IF_NOT(IsAligned<T>(objPtr), "Object is not aligned.");
    return objPtr;
  }

  template <typename T>
  T* GetAligned(T* objPtr)
  {
    if (IsAligned(objPtr))
    {
      return objPtr;
    }

    void* bufPtr = const_cast<T*>(objPtr);
    std::size_t bufSize = sizeof(T) * 2;
    auto aligned = Private::Align(std::alignment_of<T>::value, sizeof(T), bufPtr, bufSize);
    return CheckAligned(static_cast<T*>(aligned));
  }
}

#endif
