#include "StarLine.hpp"
#include "StarFormat.hpp"

using namespace Star;

int main(int argc, char** argv) {
  Line2F a = {Vec2F(-.9, 1), Vec2F(1, -1)};
  Line2F b = {Vec2F(-1, 0), Vec2F(1, 0)};

  if (a.intersects(b).intersects) {
    coutf("pass\n");
  } else {
    coutf("fail\n");
  }

  return 0;
}
