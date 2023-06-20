#include "StarPythonic.hpp"
#include "StarString.hpp"
#include "StarFormat.hpp"

using namespace Star;

int main(int argc, char** argv) {
  try {
    List<int> list = {2, 3, 4, 1, 0};
    sortByComputedValue(list, [](int i) { return i; });
    coutf("sorted list: %s\n", list);

    char something[4] = {'f', 'o', 'o', 0};
    coutf("hash1: %s\n", std::hash<char*>()(something));
    something[1] = 'u';
    coutf("hash2: %s\n", std::hash<char*>()(something));

    for (auto const& elem : zip<List>(List<int>{1, 2, 3}))
      coutf("tiny zip: %s\n", std::get<0>(elem));

    auto zipResult = zip<List>(
        List<int>{1, 2, 3},
        List<int>{5, 4, 3, 2, 1},
        List<int>{3, 2, 2 },
        List<int>{0, 0, 0, 0, 4, 8}
      );

    for (auto const& elem : zipResult)
      coutf("zip: %s %s %s %s\n", std::get<0>(elem), std::get<1>(elem), std::get<2>(elem), std::get<3>(elem));

    List<int> zla{1, 2, 3};
    List<int> zlb{5, 4, 3, 2, 1};
    List<int> zlc{3, 2, 2 };
    List<int> zld{0, 0, 0, 0, 4, 8};

    for (auto const& elem : zipIterator(zla, zlb, zlc, zld)) {
      coutf("zip: %s %s %s %s\n", std::get<0>(elem), std::get<1>(elem), std::get<2>(elem), std::get<3>(elem));
      std::get<0>(elem) = 0;
    }

    coutf("mutated zipped list a: %s\n", zla);

    for (auto const& elem : range(0, 10, 1))
      coutf("range: %s\n", elem);

    for (auto const& elem : zip<List>(range(0, 10, 1), range(0, 20, 2), range(0, 100, 5)))
      coutf("combined: %s %s %s\n", std::get<0>(elem), std::get<1>(elem), std::get<2>(elem));

    coutf("range(0, 10, 3)[5]: %s\n", range(0, 10, 3)[5]);

    StringList slist = {"foo", "bar", "baz"};
    for (auto const& pair : enumerate<List>(slist))
      coutf("enumerate output: %s %s\n", pair.first, pair.second);

  } catch (std::exception const& e) {
    cerrf("exception caught: %s\n", e.what());
    return 1;
  }

  return 0;
}
