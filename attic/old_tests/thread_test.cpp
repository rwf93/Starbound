#include "StarThread.hpp"
#include "StarFormat.hpp"

using namespace Star;

ConditionVariable cond;
Mutex mutex;

struct TestThread : Thread {
  TestThread() : Thread("TestThread") {}

  void run() {
    MutexLocker locker(mutex);
    for (int i = 0; i < 100; ++i) {
      cond.wait(mutex, 5000);
      std::cout << "stuff" << std::endl;
    }
  }
};

int main(int argc, char** argv) {
  TestThread thread;
  thread.start();

  for (int i = 0; i < 100; ++i) {
    Thread::sleep(1000);
    MutexLocker locker(mutex);
    cond.signal();
  }

  thread.join();
}
