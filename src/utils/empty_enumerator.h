#ifndef __UTILS_EMPTY_ENUMERATOR__
#define __UTILS_EMPTY_ENUMERATOR__

#include <utils/enumerator.h>

namespace Utils
{
  template <typename T>
  class EmptyEnumerator : public Enumerator<T>
  {
  public:
    using Data = typename Enumerator<T>::Data;
    using EventsObserver = typename Enumerator<T>::EventsObserver;

  public:
    virtual void SetObserver(EventsObserver) override
    {
    }

    virtual bool Next(Data&) override
    {
      return false;
    }
  };
}

#endif
