#ifndef __UTILS_MERGE_SORT_H__
#define __UTILS_MERGE_SORT_H__

#include <cstring>
#include <type_traits>

namespace Utils
{
  template <typename T, typename Less>
  void MergeSort(T* buf, T* arr, std::size_t first, std::size_t last, Less less)
  {
    //static_assert(std::is_pod<T>::value, "POD type is required.");

    if (first < last)
    {
      const auto middle = (first + last) / 2;

      MergeSort(buf, arr, first, middle, less);
      MergeSort(buf, arr, middle + 1, last, less);

      std::memcpy(&buf[first], &arr[first], (last - first + 1) * sizeof(*buf));

      auto i = first;
      auto j = first;
      auto k = middle + 1u;

      while (i <= middle && k <= last)
      {
        if (less(buf[i], buf[k]))
        {
          arr[j++] = buf[i++];
        }
        else
        {
          arr[j++] = buf[k++];
        }
      }

      std::memcpy(&arr[j], &buf[i], (middle - i + 1) * sizeof(*buf));
    }
  }

  template <typename T>
  void MergeSort(T* buf, T* arr, std::size_t first, std::size_t last)
  {
    MergeSort(buf, arr, first, last, [](const auto& lhs, const auto& rhs)
    {
      return lhs < rhs;
    });
  }
}

#endif
