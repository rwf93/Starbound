#include "StarPoly.hpp"
#include "StarFormat.hpp"

using namespace Star;

int main(int argc, char** argv) {
  Line2F line = {Vec2F(-1.87231, -0.557495), Vec2F(-0.20874, -0.455933)};
  PolyF poly;
  poly.add(Vec2F(-0.375, -1.5));
  poly.add(Vec2F(0.375, -1.5));
  poly.add(Vec2F(0.375, 0.125));
  poly.add(Vec2F(-0.375, 0.125));

  if (poly.intersects(line)) {
    coutf("Intersects\n");
  } else {
    coutf("No intersection\n");
  }
}
