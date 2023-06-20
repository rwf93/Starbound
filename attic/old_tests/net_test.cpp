#include "StarNetStates.hpp"

using namespace Star;

void testStepSendReceive() {
  NetSchema schema;

  schema.addFixedPointField("position_x", NetType::VarInt, 0.1);
  schema.addFixedPointField("position_y", NetType::VarInt, 0.1);
  schema.finalize();

  NetStepSender sender(schema);

  sender.setStep(0);
  sender.setFloat("position_x", 100.783);
  sender.setFloat("position_y", 50.134);
  ByteArray s1 = sender.writeFull();
  coutf("sent packet of %s bytes\n", s1.size());

  sender.setStep(1);
  sender.setFloat("position_x", 100.5);
  sender.setFloat("position_y", 49.938);
  ByteArray s2 = sender.writeDelta(0);
  coutf("sent packet of %s bytes\n", s2.size());

  sender.setStep(2);
  sender.setFloat("position_x", 100.289);
  ByteArray s3 = sender.writeDelta(1);
  coutf("sent packet of %s bytes\n", s3.size());

  NetReceiver receiver(schema);

  receiver.readFull(s1);
  coutf("First value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));

  receiver.readDelta(s2);
  coutf("Second value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));

  receiver.readDelta(s3);
  coutf("Third value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));
}

void testSyncSendReceive() {
  DataStreamBuffer bufferStream;
  NetSchema schema;

  schema.addFixedPointField("position_x", NetType::VarInt, 0.1);
  schema.addFixedPointField("position_y", NetType::VarInt, 0.1);
  schema.finalize();

  NetSyncSender sender(schema);

  sender.setFloat("position_x", 100.783);
  sender.setFloat("position_y", 50.134);
  size_t s1 = sender.writeFull(bufferStream);
  coutf("sent packet of %s bytes\n", s1);

  sender.setFloat("position_x", 100.5);
  sender.setFloat("position_y", 49.938);
  size_t s2 = sender.writeDelta(bufferStream);
  coutf("sent packet of %s bytes\n", s2);

  sender.setFloat("position_x", 100.289);
  size_t s3 = sender.writeDelta(bufferStream);
  coutf("sent packet of %s bytes\n", s3);

  bufferStream.seek(0);

  NetReceiver receiver(schema);

  receiver.readFull(bufferStream, s1);
  coutf("First value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));

  receiver.readDelta(bufferStream, s2);
  coutf("Second value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));

  receiver.readDelta(bufferStream, s3);
  coutf("Third value received (%s, %s)\n", receiver.getFloat("position_x"), receiver.getFloat("position_y"));
}

int main(int argc, char** argv) {
  testStepSendReceive();
  testSyncSendReceive();
  return 0;
}
