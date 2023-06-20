#include "StarLiveCounter.hpp"
#include "StarMap.hpp"
#include "StarLogging.hpp"
#include "StarThread.hpp"

namespace Star {

struct LiveCounterData {
  Mutex lock;
  HashMap<std::type_index, unique_ptr<LiveAtomicCounter>> counters;
};

LiveCounterData& liveCounterData() {
  static LiveCounterData data;
  return data;
};

void bindLiveCounter(std::type_index const& typeIndex, LiveAtomicCounter*& counter) {
  if (!counter) {
    auto& data = liveCounterData();
    MutexLocker locker(data.lock);
    auto& ptr = data.counters[typeIndex];
    if (!ptr)
      ptr = make_unique<LiveAtomicCounter>();
    counter = ptr.get();
  }
}

void dumpLiveCounters() {
#ifdef STAR_ENABLE_LIVECOUNTER
  auto& data = liveCounterData();
  Logger::info("LiveCounters");
  MutexLocker locker(data.lock);
  for (auto const& e : data.counters)
    Logger::info("  %s %s", e.first.name(), e.second->load());
#endif
}

}
