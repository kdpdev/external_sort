#ifndef __UTILS_ENUMERATOR__
#define __UTILS_ENUMERATOR__

#include <utils/observer.h>

namespace Utils
{
  template <typename T>
  class Enumerator
  {
  public:
    using Data = T;
    using EventsObserver = StringsObserver;

  public:
    virtual ~Enumerator() = default;

    virtual void SetObserver(EventsObserver observer) = 0;
    virtual bool Next(Data& data) = 0;
  };
}

#endif
