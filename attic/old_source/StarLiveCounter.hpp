#ifndef STAR_LIVE_COUNTER_HPP
#define STAR_LIVE_COUNTER_HPP

#include "StarConfig.hpp"

#include <typeindex>

namespace Star {

typedef atomic<int64_t> LiveAtomicCounter;

void bindLiveCounter(std::type_index const& typeIndex, LiveAtomicCounter*& counter);
void dumpLiveCounters();

// Use as class MyClass : LiveCounter<MyClass> { ... }
template<typename T>
class LiveCounter {
public:
#ifdef STAR_ENABLE_LIVECOUNTER
  LiveCounter() {
    bindLiveCounter(typeid(T), s_liveCounter);
    ++(*s_liveCounter);
  }

  LiveCounter(LiveCounter const&) {
    bindLiveCounter(typeid(T), s_liveCounter);
    ++(*s_liveCounter);
  }

  LiveCounter(LiveCounter&&) {
    bindLiveCounter(typeid(T), s_liveCounter);
    ++(*s_liveCounter);
  }

  void operator=(LiveCounter const&) {}

  void operator=(LiveCounter&&) {}

  ~LiveCounter() {
    --(*s_liveCounter);
  }
private:
  static LiveAtomicCounter* s_liveCounter;
#endif
};

#ifdef STAR_ENABLE_LIVECOUNTER
template<typename T>
LiveAtomicCounter* LiveCounter<T>::s_liveCounter = nullptr;
#endif

}

#endif
