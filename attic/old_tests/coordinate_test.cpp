#include "StarRoot.hpp"
#include "StarCelestialDatabase.hpp"

using namespace Star;

int main(int argc, char** argv) {
  try {
    auto root = make_shared<Root>("./assets");
    auto celestialDatabase = root->celestialDatabase();

    if (argc == 2) {
      auto system = celestialDatabase->parseSystemCoordinate(argv[1]);
      coutf("Given system: %s %s\n", system, system.systemName());
    } else {
      auto randSystem = celestialDatabase->findRandomSystem();
      coutf("Random system: %s %s\n", randSystem, randSystem.systemName());
    }

    return 0;
  } catch(std::exception const& e) {
    cerrf("exception caught: %s\n", e.what()); 
    return 1;
  }
}
