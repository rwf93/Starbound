#include "StarAny.hpp"
#include "StarFormat.hpp"
#include "StarLexicalCast.hpp"

using namespace Star;

struct MyStruct {
  MyStruct() {
    i = 100;
  }

  MyStruct(MyStruct const& rhs) {
    i = rhs.i;
  }

  MyStruct(int i) : i(i) {}

  ~MyStruct() {
  }

  int i;
};

std::ostream& operator<<(std::ostream& os, MyStruct const& s) {
  os << s.i;
  return os;
}

struct ToStringer {
  template<typename T>
  String operator()(T const& op) {
    return toString(op);
  }
};

int main(int argc, char** argv) {
  try {
    coutf("Internal any policy for int: %s\n", AnyDetail::getPolicy<int>()->isInternal());
    coutf("Internal any policy for int64_t: %s\n", AnyDetail::getPolicy<uint64_t>()->isInternal());
    coutf("Internal any policy for shared_ptr<char>: %s\n", AnyDetail::getPolicy<shared_ptr<char>>()->isInternal());
    coutf("Internal any policy for shared_ptr<char const>: %s\n", AnyDetail::getPolicy<shared_ptr<char const>>()->isInternal());
    coutf("Internal any policy for char*: %s\n", AnyDetail::getPolicy<char*>()->isInternal());
    coutf("Internal any policy for char const*: %s\n", AnyDetail::getPolicy<char const*>()->isInternal());
    coutf("Internal any policy for MyStruct: %s\n", AnyDetail::getPolicy<MyStruct>()->isInternal());

    typedef AnyOf<int, char const*, MyStruct> MyAny;

    MyAny a = MyStruct{2};
    MyAny b = a;
    b = 2;
    b = a;
    a = "foo";

    coutf("Printing any values: %s %s\n", a.capply(ToStringer()), b.capply(ToStringer()));

    MyAny c;
    c.makeType(3);
    coutf("Held type %s, printing after setting to different type via makeType: %s\n", c.getType(), c.apply(ToStringer()));
    return 0;
  } catch (std::exception const& e) {
    coutf("Exception caught: %s\n", e.what());
    return 1;
  }
}
