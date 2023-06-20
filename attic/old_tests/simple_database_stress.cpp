#include "StarSimpleDatabase.hpp"
#include "StarFile.hpp"
#include "StarRandom.hpp"

using namespace Star;
using namespace std;

const size_t MaxRecordSize = 10000;
const size_t RecordCount = 10000;
const size_t BlockSize = 2048;

int main(int argc, char** argv) {
  SimpleDatabase db;
  db.setIODevice(File::open("./simple.db", File::ReadWrite | File::Truncate));
  db.setKeySize(8);
  db.setBlockSize(BlockSize);
  db.setAutoCommit(false);

  db.open();

  DataStreamBuffer key(8);
  ByteArray value(MaxRecordSize, 0);

  StreamOffset totalRecordSize = 0;

  coutf("inserting values...\n");
  for (size_t i = 0; i < RecordCount; ++i) {
    key.seek(0);
    key.write<uint32_t>(Random::randu32());
    key.write<uint32_t>(Random::randu32());
    size_t recordSize = Random::randf() * MaxRecordSize;
    db.insert(key.data(), ByteArray(value.ptr(), recordSize));
    totalRecordSize += recordSize + 8;
    if (i % 1000 == 0)
      db.commit();
  }

  coutf("done\n\n");

  db.commit();

  coutf("number of records: %s, total record size: %s\n", RecordCount, totalRecordSize);
  coutf("total block count: %s, index block count: %s\n", db.blockCount(), db.indexCount());

  StreamOffset totalFileSize = (StreamOffset)(db.blockCount()) * BlockSize + 256;
  float indexOverhead = db.indexCount() * BlockSize / (float)totalFileSize * 100;
  float totalOverhead = (totalFileSize - totalRecordSize) / (float)totalFileSize * 100;

  coutf("index overhead: %s%%, total overhead: %s%%\n", indexOverhead, totalOverhead);

  db.close();
}
