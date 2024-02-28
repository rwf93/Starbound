#include <iostream>

#include "StarBlockStorage.hpp"
#include "StarFile.hpp"
#include "StarFormat.hpp"

using namespace Star;
using namespace std;

static size_t TotalCount = 20;
static size_t FreeCount = 10;
static size_t BlockSize = 128;
static size_t TestRepeatCount = 10;

int main(int argc, char** argv) {
  BlockStoragePtr bs = std::make_shared<BlockStorage>();
  bs->setIODevice(File::open("./blocktest", File::ReadWrite | File::Truncate));
  bs->setBlockSize(BlockSize);
  bs->open();
  bs->writeToUserHeader(0, "01234567", 8);

  size_t next = 0;

  List<BlockStorage::BlockIndex> usedBlocks;
  bs->reserveBlocks(TotalCount, back_inserter(usedBlocks));
  ByteArray writeBlock(BlockSize, 0);

  cout << "writing: ";
  for (BlockStorage::BlockIndex b : usedBlocks) {
    coutf("(%s:%s) ", next, b);
    writeBlock.fill((char)next++);
    bs->updateBlock(b, writeBlock);
  }
  cout << endl;

  for (size_t repeat = 0; repeat < TestRepeatCount; ++repeat) {
    Random::shuffle(usedBlocks);
    getc(stdin);

    List<BlockStorage::BlockIndex> blocksToFree = usedBlocks.slice(0, FreeCount);
    usedBlocks.erase(0, FreeCount);

    cout << "freeing: " << blocksToFree << endl;
    bs->freeBlocks(blocksToFree.begin(), blocksToFree.end());

    getc(stdin);
    List<BlockStorage::BlockIndex> reserveBlocks;
    bs->reserveBlocks(FreeCount, back_inserter(reserveBlocks));

    cout << "writing: ";
    for (BlockStorage::BlockIndex b : reserveBlocks) {
      coutf("(%s:%s) ", next, b);
      writeBlock.fill((char)next++);
      bs->updateBlock(b, writeBlock);
      usedBlocks.append(b);
    }
    cout << endl;
  }

  getc(stdin);
  cout << "freeing all" << endl;
  bs->freeBlocks(usedBlocks.begin(), usedBlocks.end());

  getc(stdin);
  cout << "shrinking and closing" << endl;
  bs->shrink();

  bs->close();
}
