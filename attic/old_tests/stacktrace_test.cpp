#include "StarFormat.hpp"

using namespace Star;

void throwException() {
  throw StarException("some exception");
}

void baz() {
  throwException();
}

void bar() {
  baz();
}

void foo() {
  try {
    bar();
  } catch(std::exception const& e) {
    throw StarException("some top-level exception", e);
  }
}

int main(int argc, char** argv) {
  try {
    foo();
  } catch(std::exception const& e) {
    coutf("%s\n", e.what());
  }
}
