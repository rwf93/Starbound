#include "StarRandom.hpp"
#include "StarFormat.hpp"
#include "StarLexicalCast.hpp"

using namespace Star;

int main(int argc, char** argv) {
  try {
    uint64_t seed = 0;
    if (argc == 2)
      seed = lexicalCast<uint64_t>(argv[1]);
    RandomSource rand(seed);

    for (int i = 0; i < 10; ++i)
      coutf("%s: %s\n", i, rand.randu32());

    return 0;
  } catch (std::exception const& e) {
    coutf("Exception caught: %s\n", e.what());
    return 1;
  }
}
