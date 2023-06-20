#ifndef STAR_NET_TRANSMITTERS_HPP
#define STAR_NET_TRANSMITTERS_HPP

#include "StarNetSchema.hpp"
#include "StarDataStream.hpp"
#include "StarStepInterpolation.hpp"
#include "StarVariant.hpp"
#include "StarAny.hpp"

namespace Star {

// Send states using delta coding based on a always increasing version.  The
// version is never sent over the wire, rather it is used as a way to track
// versions of states.  Useful to send updates to several receivers.  It must
// be used in the following way to avoid lost data:
//
// 1. Set any changed data.
// 2. Do any number of writeDelta calls
// 3. Set new version
// 4. Repeat...
//
// writeFull can be called at any time, but it is generally used on the 0th
// version, so that writeDelta(0) is never called.  The order is then:
//
// 1. Set any initial data.
// 2. Do writeFull call.
// 3. Set new version > 0
// 4. Enter the loop above.
class NetVersionSender {
public:
  NetVersionSender();
  NetVersionSender(NetSchema const& schema);

  NetVersionSender(NetVersionSender const& sender) = delete;
  NetVersionSender& operator=(NetVersionSender const& sender) = delete;

  void initialize(NetSchema const& schema);
  NetSchema const& schema() const;

  // Resets version, clears events, keeps all current values.
  void resetVersion();
  // After initialization / reset, version must always go forward.
  void incrementVersion(uint64_t version);
  uint64_t currentVersion() const;

  // Marks this event as having occurred
  void triggerEvent(StringRef eventName);

  // The correct set function must be called for each data type.  Signed
  // integral values must be set with setInt, unsigned integral values with
  // setUInt.  Any floating point type *or* an integral type if marked as a
  // "fixed point value" must be set with setFloat.  Bools must be set with
  // setBool, and data values must be set with setData.  Anything else results
  // in an exception.

  void setInt(StringRef name, int64_t i);
  void setUInt(StringRef name, uint64_t u);
  void setSize(StringRef name, size_t s);
  void setFloat(StringRef name, double d);
  void setBool(StringRef name, bool b);
  void setString(StringRef name, String const& str);
  void setData(StringRef name, ByteArray const& data);
  void setVariant(StringRef name, Variant const& data);

  // Write full set of states.  Returns bytes written.
  size_t writeFull(DataStream& ds) const;
  ByteArray writeFull() const;

  // Write all the state changes that have happened since (and including)
  // sinceVersion.  Returns bytes written.  The normal way to use this would be
  // each call to writeDelta should be passed the version at the time of the
  // *last* call to writeDelta, + 1.  writeDelta with sinceVersion = 0 writes
  // all states, and is a larger version of writeFull().
  size_t writeDelta(DataStream& ds, uint64_t sinceVersion) const;
  ByteArray writeDelta(uint64_t sinceVersion) const;

private:
  typedef AnyOf<int64_t, uint64_t, size_t, double, bool, ByteArray, String, Variant> DataPoint;

  struct Value {
    uint64_t lastUpdated;
    DataPoint data;
  };
  typedef std::shared_ptr<Value> ValuePtr;

  ValuePtr const& getValue(StringRef name) const;
  size_t writeValue(DataStream& ds, NetFieldInfo const& fieldInfo, ValuePtr const& value) const;

  NetSchema m_schema;

  uint64_t m_currentVersion;

  Map<unsigned, ValuePtr> m_values;
  StringMap<ValuePtr> m_valuesByName;
};

// Like NetVersionSender, except no concept of version, just sends only when
// values have been changed in between writes.  Only works when writing to a
// single receiver.  Is binary compatible with NetVersionSender.
class NetSyncSender: public NetVersionSender {
public:
  NetSyncSender();
  NetSyncSender(NetSchema const& schema);

  // Write all of the states.
  size_t writeFull(DataStream& ds);

  // Write all the state changes that have occurred.  It is possible to use
  // this without ever using writeFull (though it may transfer slightly more
  // data the first time).
  size_t writeDelta(DataStream& ds);

  ByteArray writeFull();
  ByteArray writeDelta();

private:
  using NetVersionSender::incrementVersion;
  using NetVersionSender::currentVersion;
  using NetVersionSender::writeFull;
  using NetVersionSender::writeDelta;
};

// Receive states using delta coding and optional smoothing from either a
// NetVersionSender or a NetSyncSender
class NetReceiver {
public:
  typedef std::function<double(double, double)> DifferenceFunction;

  NetReceiver();
  NetReceiver(NetSchema const& schema);

  NetReceiver(NetReceiver const& receiver) = delete;
  NetReceiver& operator=(NetReceiver const& receiver) = delete;

  void initialize(NetSchema const& schema);

  NetSchema const& schema() const;

  // Enables interpolation mode for this NetReceiver.  If interpolation mode is
  // enabled, then all states are marked with a floating step value, and states
  // are delayed until the interpolation step reaches the given step value for
  // that state.  This also enables optional smoothing on floating point values
  // that looks ahead into the future to even out changes, and can also
  // optionally extrapolate into the future if no data is available.
  void enableInterpolation(double startStep, double extrapolation = 0.0);
  void disableInterpolation();

  // For certain values, simple subtraction between two values may not be
  // appropriate for finding the difference between them (such as circular
  // values).  If provided, this function will be called instead.
  void setInterpolationDiffFunction(StringRef name, DifferenceFunction diffFunction = DifferenceFunction());

  // If using interpolation, should be called every step.  Throws away old
  // data, so if this is called with a future value then goes backwards, it may
  // result in different behavior (the interpolated data will be locked to the
  // highest time value given here).
  void setInterpolationStep(double step);

  // Read full state list sent by a NetVersionSender.
  void readFull(DataStream& ds, size_t dataSize);
  void readFull(ByteArray const& fullState);

  // Read partial state list sent by a NetVersionSender.  If interpolation is
  // enabled, interpolationStep must be specified.  interpolationStep is
  // assumed to monotonically increase here, and even in the event that it goes
  // backwards, the data provided most recently will still be treated as though
  // it is the best, most recent data.
  void readDelta(DataStream& ds, size_t dataSize, double interpolationStep = 0);
  void readDelta(ByteArray const& deltaState, double interpolationStep = 0);

  // If no data for this timestep is available, treat it as though no data has
  // actually changed, thus preventing extrapolation.  Only has an effect if
  // interpolation is in use.
  void interpolationHeartbeat(double interpolationStep);

  // Has the current value of this field been read via get*?  (Useful to avoid
  // a costly method on state change.)  Safe to ignore completely.
  bool updated(StringRef name) const;

  // Has the given even occurred since the last time eventOccurred was called
  // for this event?  Right now, events can only be triggered as fast as
  // NetStates updates happen.  Multiple calls to triggerEvent between updates
  // will only result in this returning true once.
  bool eventOccurred(StringRef eventName);

  int64_t getInt(StringRef name);
  uint64_t getUInt(StringRef name);
  size_t getSize(StringRef name);
  double getFloat(StringRef name, bool interpolateIfAvailable = true);
  bool getBool(StringRef name);
  String getString(StringRef name);
  ByteArray getData(StringRef name);
  Variant getVariant(StringRef name);

  // Forward all states from a receiver to another sender.  This is useful to
  // tie one sender to multiple chained receivers.  Data is forwarded
  // immediately as it is received, not as it is reached via interpolation.
  void forward(NetVersionSender& sender, bool forceAll = false);

private:
  typedef AnyOf<int64_t, uint64_t, size_t, double, bool, ByteArray, String, Variant> DataPoint;

  struct Value {
    // Absolute Latest data.
    DataPoint latestData;
    bool needsForwarding;

    // Best, most recent *exact* data point, without looking into the future.
    DataPoint exactData;
    bool exactUpdated;

    // Interpolation stream
    StepStream<DataPoint> stream;
    DifferenceFunction diffFunction;
  };
  typedef std::shared_ptr<Value> ValuePtr;

  ValuePtr const& getValue(StringRef name) const;
  size_t readValue(DataStream& ds, NetFieldInfo const& fieldInfo, ValuePtr const& value, double step);

  NetSchema m_schema;
  Map<unsigned, ValuePtr> m_values;
  StringMap<ValuePtr> m_valuesByName;

  bool m_interpolationEnabled;
  double m_interpolationStep;
};

}

#endif
