#include <cstring>

#include "StarCompression.hpp"
#include "StarFormat.hpp"
#include "StarMap.hpp"

using namespace Star;

int main(int argc, char** argv) {
  char const* testData =
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab"
    "ababababababababababababababababababababababababab";

  ByteArray compressedData = compressData(ByteArray(testData, strlen(testData)));
  coutf("compressed data size %s\n", compressedData.size());

  ByteArray uncompressedData = uncompressData(compressedData);
  coutf("uncompressed data size %s\n", uncompressedData.size());

  return 0;
}
