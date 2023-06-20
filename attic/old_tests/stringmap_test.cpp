#include "StarStringMap.hpp"

using namespace Star;

int main(int argc, char** argv) {
  try {
    // To test, we don't want the ptrs to remain valid
    char keytmp[4];

    StringMap<int> map;
    {
      StringMap<int> mapCopy;

      strcpy(keytmp, "foo");
      map.insert(keytmp, 1);

      strcpy(keytmp, "boo");
      map.insert(keytmp, 2);

      strcpy(keytmp, "bob");
      map.insert(keytmp, 3);

      strcpy(keytmp, "fab");
      map.insert(keytmp, 4);

      map = mapCopy;

      // Not making it in, using to test operator= and that refs remain valid,
      // etc.
      strcpy(keytmp, "som");
      mapCopy.insert(keytmp, 5);
    }

    coutf("total keys: %s\n", map.size());
    coutf("%s %s %s %s\n", map.get("foo"), map.get(String("boo")), map.get("bob"), map.get(String("fab")));

    map.remove("foo");
    map.remove("bob");
    coutf("%s %s\n", map.get("boo"), map.get("fab"));
  } catch (std::exception const& e) {
    coutf("ERROR: Exception caught! %s\n", e.what());
  }
}
