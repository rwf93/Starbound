#include "StarCasting.hpp"

using namespace Star;

struct ClassA {
  virtual ~ClassA() {}
};

struct ClassB : ClassA {
  ClassB(int member) : member(member) {}
  virtual ~ClassB() {}
  int member;
};

int main(int argc, char** argv) {
  ClassA* ptra = new ClassA();
  ClassA* ptrb = new ClassB(42);

  coutf("%s\n", is<ClassB>(ptra));
  coutf("%s\n", is<ClassB>(ptrb));
  coutf("%s\n", convert<ClassB>(ptrb)->member);

  try {
    // Expected to throw exception
    coutf("%s\n", convert<ClassB>(ptra)->member);
  } catch (std::exception const& e) {
    coutf("Expected exception caught: %s\n", e.what());
  }

  shared_ptr<ClassA> sharedptra = make_shared<ClassA>();
  shared_ptr<ClassA> sharedptrb = make_shared<ClassB>(42);

  coutf("%s\n", is<ClassB>(sharedptra));
  coutf("%s\n", is<ClassB>(sharedptrb));
  coutf("%s\n", convert<ClassB>(sharedptrb)->member);

  try {
    // Expected to throw exception
    coutf("%s\n", convert<ClassB>(sharedptra)->member);
  } catch (std::exception const& e) {
    coutf("Expected exception caught: %s\n", e.what());
  }

  delete ptra;
  delete ptrb;

  return 0;
}
