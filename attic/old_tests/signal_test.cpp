#include "StarSignalHandler.hpp"
#include "StarFormat.hpp"

using namespace Star;

void causeSegfault() {
  volatile int* f = nullptr;
  coutf("%s\n", *f);
}

int main() {
  SignalHandler signalHandler;
  causeSegfault();
  return 0;
}
