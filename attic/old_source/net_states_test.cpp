#include "StarNetStates.hpp"
#include "StarNetStepElements.hpp"
#include "StarDataStreamExtra.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(NetSyncStates, Values) {
  enum class Test {
    Value1,
    Value2,
    Value3
  };

  NetStatesEnum<Test> masterField1(Test::Value2);
  NetSyncStates master(masterField1);

  EXPECT_EQ(masterField1.get(), Test::Value2);
  masterField1.set(Test::Value3);
  EXPECT_EQ(masterField1.get(), Test::Value3);
}

TEST(NetSyncStates, DataRounding) {
  auto masterField1 = NetStatesUInt::makeUInt8();
  auto masterField2 = NetStatesUInt::makeUInt16();
  auto masterField3 = NetStatesFloat::makeNormalizedFloat8();
  auto masterField4 = NetStatesFloat::makeFixedPoint16(0.1);
  auto masterField5 = NetStatesFloat::makeFixedPoint16(0.5);
  auto masterField6 = NetStatesFloat::makeNormalizedFloat16();
  auto masterField7 = NetStatesFloat::makeFixedPoint8(0.1);
  NetSyncStates master(masterField1, masterField2, masterField3, masterField4, masterField5, masterField6, masterField7);

  masterField1.set(300);
  masterField2.set(100000);
  masterField3.set(1.5);
  masterField4.set(2.1999);
  masterField5.set(100000);
  masterField6.set(0.777777);
  masterField7.set(0.999);

  // Check the master side value bounds limits

  EXPECT_EQ(masterField1.get(), 255u);
  EXPECT_EQ(masterField2.get(), 65535u);
  EXPECT_LT(fabs(masterField3.get() - 1.0), 0.00001);
  EXPECT_LT(fabs(masterField5.get() - 16383.5), 0.00001);

  // Check to make sure encoded data is actually sent with the expected
  // limitations to the client side.

  auto slaveField1 = NetStatesUInt::makeUInt8();
  auto slaveField2 = NetStatesUInt::makeUInt16();
  auto slaveField3 = NetStatesFloat::makeNormalizedFloat8();
  auto slaveField4 = NetStatesFloat::makeFixedPoint16(0.1);
  auto slaveField5 = NetStatesFloat::makeFixedPoint16(0.5);
  auto slaveField6 = NetStatesFloat::makeNormalizedFloat16();
  auto slaveField7 = NetStatesFloat::makeFixedPoint8(0.1);
  NetSyncStates slave(slaveField1, slaveField2, slaveField3, slaveField4, slaveField5, slaveField6, slaveField7);

  slave.readDeltaPacket(master.writeDeltaPacket());

  EXPECT_EQ(slaveField1.get(), 255u);
  EXPECT_EQ(slaveField2.get(), 65535u);
  EXPECT_LT(fabs(slaveField3.get() - 1.0), 0.00001);
  EXPECT_LT(fabs(slaveField4.get() - 2.2), 0.00001);
  EXPECT_LT(fabs(slaveField5.get() - 16383.5), 0.00001);
  EXPECT_LT(fabs(slaveField6.get() - 0.777782), 0.000001);
  EXPECT_LT(fabs(slaveField7.get() - 1.0), 0.000001);

  // Make sure that jittering a fixed point or limited value doesn't cause
  // extra deltas

  masterField1.set(256);
  masterField2.set(65536);
  masterField4.set(2.155);
  masterField4.set(2.24);

  EXPECT_FALSE(master.hasDelta());
}

TEST(NetStepStates, DirectWriteRead) {
  auto masterField1 = NetStatesUInt::makeVarUInt(1);
  auto masterField2 = NetStatesUInt::makeVarUInt(2);
  auto masterField3 = NetStatesUInt::makeVarUInt(3);
  auto masterField4 = NetStatesUInt::makeVarUInt(4);
  NetStepStates master(masterField1, masterField2, masterField3, masterField4);

  auto slaveField1 = NetStatesUInt::makeVarUInt();
  auto slaveField2 = NetStatesUInt::makeVarUInt();
  auto slaveField3 = NetStatesUInt::makeVarUInt();
  auto slaveField4 = NetStatesUInt::makeVarUInt();
  NetStepStates slave(slaveField1, slaveField2, slaveField3, slaveField4);

  DataStreamBuffer ds;
  master.writeFull(ds);
  ds.seek(0);
  slave.readFull(ds);

  EXPECT_EQ(slaveField1.get(), 1u);
  EXPECT_EQ(slaveField2.get(), 2u);
  EXPECT_EQ(slaveField3.get(), 3u);
  EXPECT_EQ(slaveField4.get(), 4u);

  master.updateStep(1);
  masterField1.set(10);
  masterField3.set(30);

  ds.clear();
  master.writeDelta(ds, 1);
  ds.seek(0);
  slave.readDelta(ds, 1);

  EXPECT_EQ(slaveField1.get(), 10u);
  EXPECT_EQ(slaveField2.get(), 2u);
  EXPECT_EQ(slaveField3.get(), 30u);
  EXPECT_EQ(slaveField4.get(), 4u);

  master.updateStep(2);
  masterField2.set(20);
  masterField4.set(40);

  ds.clear();
  master.writeDelta(ds, 2);
  ds.seek(0);
  slave.readDelta(ds, 2);

  EXPECT_EQ(slaveField1.get(), 10u);
  EXPECT_EQ(slaveField2.get(), 20u);
  EXPECT_EQ(slaveField3.get(), 30u);
  EXPECT_EQ(slaveField4.get(), 40u);
}

TEST(NetSyncStates, MasterSlave) {
  auto masterField1 = NetStatesInt::makeInt8();
  auto masterField2 = NetStatesUInt::makeUInt16();
  auto masterField3 = NetStatesUInt::makeVarUInt();
  auto masterField4 = NetStatesSize();
  NetSyncStates master(masterField1, masterField2, masterField3, masterField4);

  auto slaveField1 = NetStatesInt::makeInt8();
  auto slaveField2 = NetStatesUInt::makeUInt16();
  auto slaveField3 = NetStatesUInt::makeVarUInt();
  auto slaveField4 = NetStatesSize();
  NetSyncStates slave(slaveField1, slaveField2, slaveField3, slaveField4);

  masterField1.set(10);
  masterField2.set(20);
  masterField3.set(30);
  masterField4.set(40);

  EXPECT_EQ(masterField1.get(), 10);
  EXPECT_EQ(masterField2.get(), 20u);
  EXPECT_EQ(masterField3.get(), 30u);
  EXPECT_EQ(masterField4.get(), 40u);

  auto delta = master.writeDeltaPacket();

  // Initial delta should be at least 9 bytes, from the 1 2-byte value, 3 >= 1
  // byte value and indexes.
  EXPECT_GE(delta.size(), 9u);

  slave.readDeltaPacket(delta);
  EXPECT_EQ(slaveField1.get(), 10);
  EXPECT_EQ(slaveField2.get(), 20u);
  EXPECT_EQ(slaveField3.get(), 30u);
  EXPECT_EQ(slaveField4.get(), 40u);

  masterField1.set(50);
  delta = master.writeDeltaPacket();

  // Second delta should be not include any other data than the single 1 byte
  // changed state, so make sure that it is <= 2 bytes.
  EXPECT_LE(delta.size(), 2u);

  slave.readDeltaPacket(delta);
  EXPECT_EQ(slaveField1.get(), 50);

  master.reset();
  delta = master.writeDeltaPacket();

  // Delta after calling resetStep should contain all values.
  EXPECT_GE(delta.size(), 9u);
}

TEST(NetSyncStates, CustomData) {
  NetStatesData<Vec2F> masterField1;
  NetSyncStates master(masterField1);

  NetStatesData<Vec2F> slaveField1;
  NetSyncStates slave(slaveField1);

  masterField1.set(Vec2F(1, 2));

  slave.readDeltaPacket(master.writeDeltaPacket());
  EXPECT_TRUE(slaveField1.get() == Vec2F(1, 2));

  masterField1.update([](Vec2F& v) {
      v = {3.0f, 4.0f};
      return true;
    });

  slave.readDeltaPacket(master.writeDeltaPacket());
  EXPECT_TRUE(slaveField1.get() == Vec2F(3, 4));
}

TEST(NetSyncStates, Forwarding) {
  auto masterField = NetStatesInt::makeInt16();
  NetSyncStates master(masterField);

  auto forwarderField = NetStatesInt::makeInt16();
  NetSyncStates forwarder(forwarderField);

  auto slaveField = NetStatesInt::makeInt16();
  NetSyncStates slave(slaveField);

  masterField.set(413);
  forwarder.readDeltaPacket(master.writeDeltaPacket());
  slave.readDeltaPacket(forwarder.writeDeltaPacket());

  EXPECT_EQ(forwarderField.get(), 413);
  EXPECT_EQ(slaveField.get(), 413);
}

TEST(NetStepStates, Forwarding) {
  auto masterField1 = NetStatesInt::makeInt16();
  auto masterField2 = NetStatesInt::makeInt16();
  auto masterField3 = NetStatesInt::makeInt16();
  auto masterField4 = NetStatesInt::makeInt16();
  NetStepStates master(masterField1, masterField2, masterField3, masterField4);

  auto forwarderField1 = NetStatesInt::makeInt16();
  auto forwarderField2 = NetStatesInt::makeInt16();
  auto forwarderField3 = NetStatesInt::makeInt16();
  auto forwarderField4 = NetStatesInt::makeInt16();
  NetStepStates forwarder(forwarderField1, forwarderField2, forwarderField3, forwarderField4);

  auto slaveField1 = NetStatesInt::makeInt16();
  auto slaveField2 = NetStatesInt::makeInt16();
  auto slaveField3 = NetStatesInt::makeInt16();
  auto slaveField4 = NetStatesInt::makeInt16();
  NetStepStates slave(slaveField1, slaveField2, slaveField3, slaveField4);

  // First emulate store / load that would happen in entity initialization.

  masterField1.set(10);
  masterField2.set(20);
  masterField3.set(30);
  masterField4.set(40);

  forwarder.readFullPacket(master.writeFullPacket());
  slave.readFullPacket(forwarder.writeFullPacket());

  EXPECT_EQ(forwarderField1.get(), 10);
  EXPECT_EQ(forwarderField2.get(), 20);
  EXPECT_EQ(forwarderField3.get(), 30);
  EXPECT_EQ(forwarderField4.get(), 40);

  EXPECT_EQ(slaveField1.get(), 10);
  EXPECT_EQ(slaveField2.get(), 20);
  EXPECT_EQ(slaveField3.get(), 30);
  EXPECT_EQ(slaveField4.get(), 40);

  // Then, update one step and transmit that step to the forwarder.

  master.updateStep(1);
  masterField1.set(413);

  // Write delta since and including step 1 (the update to "test1")
  ByteArray delta = master.writeDeltaPacket(1);
  // Make sure that the delta includes only one changed state.
  EXPECT_LT(delta.size(), 5u);

  forwarder.updateStep(1);
  EXPECT_TRUE(forwarder.readDeltaPacket(delta));

  EXPECT_EQ(forwarderField1.get(), 413);

  // Write forwarded delta since and including step 1.
  delta = forwarder.writeDeltaPacket(1);
  // Make sure that the delta includes only one changed state.
  EXPECT_LT(delta.size(), 5u);

  // Make sure that forwarded state makes it all the way to the slave.
  slave.updateStep(1);
  EXPECT_TRUE(slave.readDeltaPacket(delta));
  EXPECT_EQ(slaveField1.get(), 413);

  EXPECT_EQ(forwarderField1.get(), 413);
  EXPECT_EQ(forwarderField2.get(), 20);
  EXPECT_EQ(forwarderField3.get(), 30);
  EXPECT_EQ(forwarderField4.get(), 40);

  EXPECT_EQ(slaveField1.get(), 413);
  EXPECT_EQ(slaveField2.get(), 20);
  EXPECT_EQ(slaveField3.get(), 30);
  EXPECT_EQ(slaveField4.get(), 40);
}

TEST(NetStepStates, MasterSetGet) {
  // Make sure that Master mode sets, gets, pullStateUpdated, and pullEventOccurred
  // work properly.

  auto masterField1 = NetStatesInt::makeInt16();
  auto masterField2 = NetStatesString();
  auto masterField3 = NetStatesEvent();
  NetStepStates master(masterField1, masterField2, masterField3);

  EXPECT_EQ(masterField3.pullOccurrences(), 0u);

  masterField1.set(10);
  masterField2.set("foo");

  EXPECT_EQ(masterField1.pullUpdated(), true);
  EXPECT_EQ(masterField2.pullUpdated(), true);

  EXPECT_EQ(masterField1.pullUpdated(), false);
  EXPECT_EQ(masterField2.pullUpdated(), false);

  masterField3.trigger();
  masterField3.trigger();
  EXPECT_EQ(masterField3.pullOccurrences(), 2u);
  EXPECT_EQ(masterField3.pullOccurrences(), 0u);
}

TEST(NetStepStates, IsolatedSetGet) {
  auto masterField1 = NetStatesInt::makeInt16();
  auto masterField2 = NetStatesString();
  auto masterField3 = NetStatesEvent();

  EXPECT_EQ(masterField3.pullOccurrences(), 0u);

  masterField1.set(10);
  masterField2.set("foo");

  EXPECT_EQ(masterField1.pullUpdated(), true);
  EXPECT_EQ(masterField2.pullUpdated(), true);

  EXPECT_EQ(masterField1.pullUpdated(), false);
  EXPECT_EQ(masterField2.pullUpdated(), false);

  masterField3.trigger();
  masterField3.trigger();
  EXPECT_EQ(masterField3.pullOccurrences(), 2u);
  EXPECT_EQ(masterField3.pullOccurrences(), 0u);

  masterField1.set(20);
  masterField2.set("bar");

  masterField3.trigger();
  masterField3.trigger();

  NetStepStates master(masterField1, masterField2, masterField3);

  EXPECT_EQ(masterField1.get(), 20);
  EXPECT_EQ(masterField2.get(), "bar");

  EXPECT_EQ(masterField1.pullUpdated(), true);
  EXPECT_EQ(masterField2.pullUpdated(), true);

  EXPECT_EQ(masterField1.pullUpdated(), false);
  EXPECT_EQ(masterField2.pullUpdated(), false);

  EXPECT_EQ(masterField3.pullOccurrences(), 2u);
  EXPECT_EQ(masterField3.pullOccurrences(), 0u);
}

TEST(NetStepStates, EventTest) {
  auto masterField1 = NetStatesInt::makeVarInt();
  auto masterField2 = NetStatesEvent();
  NetStepStates master(masterField1, masterField2);

  auto slaveField1 = NetStatesInt::makeVarInt();
  auto slaveField2 = NetStatesEvent();
  NetStepStates slave(slaveField1, slaveField2);

  // Fields should always start with pullUpdatd returning true, events should
  // always start with occurred false.
  EXPECT_EQ(slaveField1.pullUpdated(), true);
  EXPECT_EQ(slaveField2.pullOccurrences(), 0u);

  masterField2.trigger();
  EXPECT_TRUE(slave.readDeltaPacket(master.writeDeltaPacket(0)));

  EXPECT_EQ(slaveField2.pullOccurrences(), 1u);
  EXPECT_EQ(slaveField2.pullOccurrences(), 0u);

  EXPECT_FALSE(master.updateStep(1));
  // Delta should be empty, nothing happened on step 1.
  EXPECT_EQ(master.hasDelta(1), false);
  auto delta = master.writeDeltaPacket(1);
  EXPECT_TRUE(delta.empty());

  EXPECT_FALSE(slave.updateStep(1));
  EXPECT_FALSE(slave.readDeltaPacket(delta));
  EXPECT_EQ(slaveField2.pullOccurrences(), 0u);

  EXPECT_FALSE(master.updateStep(2));
  masterField2.trigger();
  EXPECT_EQ(master.hasDelta(2), true);
  delta = master.writeDeltaPacket(2);

  EXPECT_FALSE(slave.updateStep(2));
  EXPECT_TRUE(slave.readDeltaPacket(delta));
  EXPECT_EQ(slaveField2.pullOccurrences(), 1u);
  EXPECT_EQ(slaveField2.pullOccurrences(), 0u);
}

TEST(NetSyncStates, Resetting) {
  auto masterField1 = NetStatesString("foo");
  auto masterField2 = NetStatesInt::makeVarInt();
  auto masterField3 = NetStatesEvent();
  NetSyncStates master(masterField1, masterField2, masterField3);

  auto slaveField1 = NetStatesString();
  auto slaveField2 = NetStatesInt::makeVarInt();
  auto slaveField3 = NetStatesEvent();
  NetSyncStates slave(slaveField1, slaveField2, slaveField3);

  EXPECT_EQ(slaveField1.pullUpdated(), true);
  EXPECT_EQ(slaveField2.pullUpdated(), true);

  masterField3.trigger();

  master.reset();
  slave.readDeltaPacket(master.writeDeltaPacket());

  EXPECT_EQ(slaveField1.pullUpdated(), true);
  EXPECT_EQ(slaveField1.get(), "foo");
  EXPECT_EQ(slaveField2.pullUpdated(), false);

  // We have triggered an event and called stepReset, the event should still
  // come through
  EXPECT_EQ(masterField3.pullOccurrences(), 1u);
  EXPECT_EQ(slaveField3.pullOccurrences(), 1u);

  masterField1.set("foo");
  slave.readDeltaPacket(master.writeDeltaPacket());
  EXPECT_EQ(slaveField1.pullUpdated(), false);

  masterField1.set("bar");
  slave.readDeltaPacket(master.writeDeltaPacket());
  EXPECT_EQ(slaveField1.pullUpdated(), true);
  EXPECT_EQ(slaveField1.get(), "bar");

  masterField1.push("bar");
  slave.readDeltaPacket(master.writeDeltaPacket());
  EXPECT_EQ(slaveField1.pullUpdated(), true);
  EXPECT_EQ(slaveField1.get(), "bar");

  masterField3.trigger();
  masterField3.trigger();
  slave.readDeltaPacket(master.writeDeltaPacket());
  slaveField3.ignoreOccurrences();
  // occurrence should not come through after "ignoreOccurrences"
  EXPECT_EQ(slaveField3.pullOccurrences(), 0u);
  EXPECT_EQ(masterField3.pullOccurrences(), 2u);

  masterField3.trigger();
  masterField3.trigger();
  masterField3.ignoreOccurrences();
  slave.readDeltaPacket(master.writeDeltaPacket());

  // ignoreOccurrences is LOCAL only, so events should still go through to the
  // slave
  EXPECT_EQ(masterField3.pullOccurrences(), 0u);

  EXPECT_EQ(slaveField3.pullOccurrences(), 2u);
  EXPECT_EQ(slaveField3.pullOccurrences(), 0u);
}

TEST(NetStepStates, Interpolation) {
  auto masterField1 = NetStatesFloat::makeFloat(1.0f);
  auto masterField2 = NetStatesBool(true);
  NetStepStates master(masterField1, masterField2);

  auto slaveField1 = NetStatesFloat::makeFloat();
  slaveField1.setInterpolator(NetStatesLerpInterpolator);
  auto slaveField2 = NetStatesBool();
  NetStepStates slave(slaveField1, slaveField2);

  slave.enableInterpolation(0);
  EXPECT_TRUE(slave.readDeltaPacket(master.writeDeltaPacket(0), 0.0));

  EXPECT_FALSE(master.updateStep(2));
  masterField1.set(2.0f);
  masterField2.set(false);
  EXPECT_FALSE(slave.readDeltaPacket(master.writeDeltaPacket(1), 2.0));

  EXPECT_FALSE(master.updateStep(4));
  masterField1.set(3.0f);
  masterField2.set(true);
  EXPECT_FALSE(slave.readDeltaPacket(master.writeDeltaPacket(3), 4.0));

  EXPECT_FALSE(master.updateStep(6));
  masterField1.set(4.0f);
  masterField2.set(false);
  EXPECT_FALSE(slave.readDeltaPacket(master.writeDeltaPacket(5), 6.0));

  EXPECT_TRUE(fabs(slaveField1.get() - 1.0) < 0.001);
  EXPECT_EQ(slaveField2.get(), true);
  EXPECT_EQ(slaveField2.pullUpdated(), true);
  EXPECT_EQ(slaveField2.get(), true);

  EXPECT_TRUE(slave.updateStep(1));
  EXPECT_TRUE(fabs(slaveField1.get() - 1.5) < 0.001);
  EXPECT_EQ(slaveField2.get(), true);
  EXPECT_EQ(slaveField2.pullUpdated(), false);

  EXPECT_TRUE(slave.updateStep(2));
  EXPECT_TRUE(fabs(slaveField1.get() - 2.0) < 0.001);
  EXPECT_EQ(slaveField2.get(), false);
  EXPECT_EQ(slaveField2.pullUpdated(), true);

  EXPECT_TRUE(slave.updateStep(3));
  EXPECT_TRUE(fabs(slaveField1.get() - 2.5) < 0.001);
  EXPECT_EQ(slaveField2.get(), false);
  EXPECT_EQ(slaveField2.pullUpdated(), false);

  EXPECT_TRUE(slave.updateStep(4));
  EXPECT_TRUE(fabs(slaveField1.get() - 3.0) < 0.001);
  EXPECT_EQ(slaveField2.get(), true);
  EXPECT_EQ(slaveField2.pullUpdated(), true);
  EXPECT_EQ(slaveField2.get(), true);

  EXPECT_TRUE(slave.updateStep(5));
  EXPECT_TRUE(fabs(slaveField1.get() - 3.5) < 0.001);
  EXPECT_EQ(slaveField2.get(), true);
  EXPECT_EQ(slaveField2.pullUpdated(), false);

  EXPECT_TRUE(slave.updateStep(6));
  EXPECT_TRUE(fabs(slaveField1.get() - 4.0) < 0.001);
  EXPECT_FALSE(slaveField2.get());
  EXPECT_EQ(slaveField2.pullUpdated(), true);
  EXPECT_EQ(slaveField2.get(), false);

  EXPECT_FALSE(slave.updateStep(7));
  EXPECT_FALSE(slave.updateStep(8));
}

TEST(NetStepStates, NetStepElements) {
  class TestElement : public NetStepElement {
  public:
    TestElement(int value = 0)
      : dataState(NetStatesInt::makeVarInt(value)), netStates(dataState) {}

    void setData(int value) {
      dataState.set(value);
    }

    int getData() const {
      return dataState.get();
    }

    ByteArray netStore() override {
      return netStates.writeFullPacket();
    }

    void netLoad(ByteArray const& store) override {
      netStates.readFullPacket(store);
    }

    void updateNetStep(uint64_t step) override {
      netStates.updateStep(step);
    }

    void resetNetStep() override {
      netStates.resetStep();
    }

    ByteArray writeNetDelta(uint64_t fromStep) override {
      return netStates.writeDeltaPacket(fromStep);
    }

    void readNetDelta(ByteArray const& data, double interpolationStep = 0.0) override {
      netStates.readDeltaPacket(data, interpolationStep);
    }

    void enableNetInterpolation(unsigned extrapolationHint) override {
      netStates.enableInterpolation(extrapolationHint);
    }

    void disableNetInterpolation() override {
      netStates.disableInterpolation();
    }

    void netInterpolationHeartbeat(double step) override {
      netStates.interpolationHeartbeat(step);
    }

  private:
    NetStatesInt dataState;
    NetStepStates netStates;
  };

  typedef NetStepDynamicElementGroup<TestElement> TestElementGroup;

  TestElementGroup masterGroup;
  TestElementGroup slaveGroup;

  auto objId1 = masterGroup.addElement(make_shared<TestElement>(1000));
  auto objId2 = masterGroup.addElement(make_shared<TestElement>(2000));

  EXPECT_EQ(masterGroup.elementIds().size(), 2u);
  EXPECT_EQ(masterGroup.getElement(objId1)->getData(), 1000);
  EXPECT_EQ(masterGroup.getElement(objId2)->getData(), 2000);

  slaveGroup.netLoad(masterGroup.netStore());

  EXPECT_EQ(slaveGroup.elementIds().size(), 2u);
  EXPECT_EQ(slaveGroup.getElement(objId1)->getData(), 1000);
  EXPECT_EQ(slaveGroup.getElement(objId2)->getData(), 2000);

  masterGroup.updateNetStep(2);
  masterGroup.getElement(objId1)->setData(1001);
  masterGroup.getElement(objId2)->setData(2001);

  slaveGroup.updateNetStep(2);
  slaveGroup.readNetDelta(masterGroup.writeNetDelta(1), 2);

  EXPECT_EQ(slaveGroup.getElement(objId1)->getData(), 1001);
  EXPECT_EQ(slaveGroup.getElement(objId2)->getData(), 2001);

  masterGroup.updateNetStep(4);
  masterGroup.getElement(objId1)->setData(1002);
  masterGroup.getElement(objId2)->setData(2002);

  slaveGroup.updateNetStep(4);
  slaveGroup.readNetDelta(masterGroup.writeNetDelta(3), 4);

  EXPECT_EQ(masterGroup.getElement(objId1)->getData(), 1002);
  EXPECT_EQ(masterGroup.getElement(objId2)->getData(), 2002);
  EXPECT_EQ(slaveGroup.getElement(objId1)->getData(), 1002);
  EXPECT_EQ(slaveGroup.getElement(objId2)->getData(), 2002);

  masterGroup.updateNetStep(6);
  auto objId3 = masterGroup.addElement(make_shared<TestElement>(3001));

  slaveGroup.updateNetStep(6);
  slaveGroup.readNetDelta(masterGroup.writeNetDelta(5));

  EXPECT_EQ(slaveGroup.elementIds().size(), 3u);
  EXPECT_EQ(slaveGroup.getElement(objId3)->getData(), 3001);

  // Delta must be greater than MaxChangeDataSteps
  masterGroup.updateNetStep(2006);
  auto objId4 = masterGroup.addElement(make_shared<TestElement>(4001));
  masterGroup.getElement(objId3)->setData(3002);
  masterGroup.getElement(objId4)->setData(4002);

  slaveGroup.updateNetStep(2006);
  slaveGroup.readNetDelta(masterGroup.writeNetDelta(7));

  EXPECT_EQ(slaveGroup.elementIds().size(), 4u);
  EXPECT_EQ(slaveGroup.getElement(objId1)->getData(), 1002);
  EXPECT_EQ(slaveGroup.getElement(objId2)->getData(), 2002);
  EXPECT_EQ(slaveGroup.getElement(objId3)->getData(), 3002);
  EXPECT_EQ(slaveGroup.getElement(objId4)->getData(), 4002);

  TestElementGroup forwardedSlaveGroup;
  forwardedSlaveGroup.readNetDelta(slaveGroup.writeNetDelta(0));
  forwardedSlaveGroup.updateNetStep(2006);

  EXPECT_EQ(forwardedSlaveGroup.elementIds().size(), 4u);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId1)->getData(), 1002);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId2)->getData(), 2002);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId3)->getData(), 3002);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId4)->getData(), 4002);

  masterGroup.updateNetStep(2008);
  masterGroup.removeElement(objId1);
  masterGroup.removeElement(objId3);
  masterGroup.getElement(objId2)->setData(2003);
  masterGroup.getElement(objId4)->setData(4003);

  slaveGroup.updateNetStep(2008);
  slaveGroup.readNetDelta(masterGroup.writeNetDelta(2007));

  forwardedSlaveGroup.updateNetStep(2008);
  forwardedSlaveGroup.readNetDelta(slaveGroup.writeNetDelta(2007));

  EXPECT_EQ(slaveGroup.elementIds().size(), 2u);
  EXPECT_EQ(slaveGroup.getElement(objId2)->getData(), 2003);
  EXPECT_EQ(slaveGroup.getElement(objId4)->getData(), 4003);

  EXPECT_EQ(forwardedSlaveGroup.elementIds().size(), 2u);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId2)->getData(), 2003);
  EXPECT_EQ(forwardedSlaveGroup.getElement(objId4)->getData(), 4003);
}

TEST(NetSyncStates, AllTypes) {
  auto masterField1 = NetStatesInt::makeInt8(9);
  auto masterField2 = NetStatesUInt::makeUInt8(7);
  auto masterField3 = NetStatesInt::makeInt16(333);
  auto masterField4 = NetStatesUInt::makeUInt16(999);
  auto masterField5 = NetStatesInt::makeVarInt(567);
  auto masterField6 = NetStatesUInt::makeVarUInt(17000);
  auto masterField7 = NetStatesSize(22222);
  auto masterField8 = NetStatesFloat::makeFloat(1.55f);
  auto masterField9 = NetStatesFloat::makeDouble(1.12345678910111213);
  auto masterField10 = NetStatesFloat::makeNormalizedFloat8(0.33333333f);
  auto masterField11 = NetStatesFloat::makeNormalizedFloat16(0.66666666f);
  auto masterField12 = NetStatesFloat::makeFixedPoint8(0.1, 1.6);
  auto masterField13 = NetStatesFloat::makeFixedPoint16(0.01, 1.62);
  auto masterField14 = NetStatesFloat::makeFixedPoint(0.01, 2000.62);
  auto masterField15 = NetStatesBool(true);
  NetSyncStates master(
      masterField1, masterField2, masterField3, masterField4, masterField5, masterField6, masterField7,
      masterField8, masterField9, masterField10, masterField11, masterField12, masterField13, masterField14,
      masterField15);
  // Events and generics are tested elsewhere

  EXPECT_EQ(masterField1.get(), 9);
  EXPECT_EQ(masterField2.get(), 7u);
  EXPECT_EQ(masterField3.get(), 333);
  EXPECT_EQ(masterField4.get(), 999u);
  EXPECT_EQ(masterField5.get(), 567);
  EXPECT_EQ(masterField6.get(), 17000u);
  EXPECT_EQ(masterField7.get(), 22222u);
  EXPECT_FLOAT_EQ(masterField8.get(), 1.55f);
  EXPECT_FLOAT_EQ(masterField9.get(), 1.12345678910111213);
  EXPECT_FLOAT_EQ(masterField10.get(), 0.33333333f);
  EXPECT_FLOAT_EQ(masterField11.get(), 0.66666666f);
  EXPECT_FLOAT_EQ(masterField12.get(), 1.6);
  EXPECT_FLOAT_EQ(masterField13.get(), 1.62);
  EXPECT_FLOAT_EQ(masterField14.get(), 2000.62);
  EXPECT_EQ(masterField15.get(), true);

  auto slaveField1 = NetStatesInt::makeInt8(9);
  auto slaveField2 = NetStatesUInt::makeUInt8(7);
  auto slaveField3 = NetStatesInt::makeInt16(333);
  auto slaveField4 = NetStatesUInt::makeUInt16(999);
  auto slaveField5 = NetStatesInt::makeVarInt(567);
  auto slaveField6 = NetStatesUInt::makeVarUInt(17000);
  auto slaveField7 = NetStatesSize(22222);
  auto slaveField8 = NetStatesFloat::makeFloat(1.55f);
  auto slaveField9 = NetStatesFloat::makeDouble(1.12345678910111213);
  auto slaveField10 = NetStatesFloat::makeNormalizedFloat8(0.33333333f);
  auto slaveField11 = NetStatesFloat::makeNormalizedFloat16(0.66666666f);
  auto slaveField12 = NetStatesFloat::makeFixedPoint8(0.1, 1.6);
  auto slaveField13 = NetStatesFloat::makeFixedPoint16(0.01, 1.62);
  auto slaveField14 = NetStatesFloat::makeFixedPoint(0.01, 2000.62);
  auto slaveField15 = NetStatesBool(true);
  NetSyncStates slave(
      slaveField1, slaveField2, slaveField3, slaveField4, slaveField5, slaveField6, slaveField7,
      slaveField8, slaveField9, slaveField10, slaveField11, slaveField12, slaveField13, slaveField14,
      slaveField15);

  slave.readDeltaPacket(master.writeDeltaPacket());

  EXPECT_EQ(slaveField1.get(), 9);
  EXPECT_EQ(slaveField2.get(), 7u);
  EXPECT_EQ(slaveField3.get(), 333);
  EXPECT_EQ(slaveField4.get(), 999u);
  EXPECT_EQ(slaveField5.get(), 567);
  EXPECT_EQ(slaveField6.get(), 17000u);
  EXPECT_EQ(slaveField7.get(), 22222u);
  EXPECT_FLOAT_EQ(slaveField8.get(), 1.55f);
  EXPECT_FLOAT_EQ(slaveField9.get(), 1.12345678910111213);
  EXPECT_NEAR(slaveField10.get(), 0.33333333f, 0.00001f);
  EXPECT_NEAR(slaveField11.get(), 0.66666666f, 0.00001f);
  EXPECT_FLOAT_EQ(slaveField12.get(), 1.6);
  EXPECT_FLOAT_EQ(slaveField13.get(), 1.62);
  EXPECT_FLOAT_EQ(slaveField14.get(), 2000.62);
  EXPECT_EQ(slaveField15.get(), true);
}

TEST(NetStepStates, FieldConnectedSemantics) {
  NetStatesString field("foo");
  EXPECT_FALSE(field.connected());
  EXPECT_EQ(field.get(), "foo");
  NetStepStates netStates;
  netStates.addField(field);
  EXPECT_TRUE(field.connected());
  EXPECT_EQ(field.get(), "foo");
  netStates.clearFields();
  EXPECT_FALSE(field.connected());
  EXPECT_EQ(field.get(), "foo");
}

TEST(NetStepStates, HashMatching) {
  auto masterField1 = NetStatesUInt::makeUInt8(5);
  auto masterField2 = NetStatesUInt::makeUInt16(10);
  auto masterField3 = NetStatesFloat::makeNormalizedFloat8();
  auto masterField4 = NetStatesFloat::makeFixedPoint16(0.1);
  auto masterField5 = NetStatesFloat::makeFixedPoint16(0.5);
  auto masterField6 = NetStatesFloat::makeNormalizedFloat16();
  NetStepStates master(masterField1, masterField2, masterField3, masterField4, masterField5, masterField6);

  NetStepStates slave;
  EXPECT_NE(master.fieldDigest(), slave.fieldDigest());

  auto slaveField1 = NetStatesUInt::makeUInt8(5);
  auto slaveField2 = NetStatesUInt::makeUInt16(10);
  auto slaveField3 = NetStatesFloat::makeNormalizedFloat8();
  auto slaveField4 = NetStatesFloat::makeFixedPoint16(0.1);
  auto slaveField5 = NetStatesFloat::makeFixedPoint16(0.5);
  auto slaveField6 = NetStatesFloat::makeNormalizedFloat16();
  slave.addFields(slaveField1, slaveField2, slaveField3, slaveField4, slaveField5, slaveField6);
  EXPECT_EQ(master.fieldDigest(), slave.fieldDigest());

  auto masterField7 = NetStatesFloat::makeFixedPoint8(0.1);
  master.addField(masterField7);
  EXPECT_NE(master.fieldDigest(), slave.fieldDigest());
  auto slaveField7 = NetStatesFloat::makeFixedPoint8(0.1);
  slave.addField(slaveField7);
  EXPECT_EQ(master.fieldDigest(), slave.fieldDigest());

  slave.readFullPacket(master.writeFullPacket());

  EXPECT_EQ(slaveField1.get(), 5u);
  EXPECT_EQ(slaveField2.get(), 10u);

  slave.addField(NetStatesInt::makeInt8());
  EXPECT_THROW(slave.readFullPacket(master.writeFullPacket()), NetStatesException);
}
