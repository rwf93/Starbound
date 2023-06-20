#include "StarLinq.hpp"

using namespace Star;

int main(int argc, char** argv) {

  List<int> src = {1,2,3,4,5,6,7,8};
  List<int> dst = Linq::from(src).where([](int a){return a%2 == 1;})    // 1,3,5,7
                      .select([](int a){return a*2;})         // 2,6,10,14
                      .where( [](int a){return a>2 && a<12;}) // 6,10
                      .toStarList();

  coutf("%s\n", dst);

  return 0;
}
