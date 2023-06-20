#ifndef STAR_NET_STATES_HPP
#define STAR_NET_STATES_HPP

#include "StarAlgorithm.hpp"
#include "StarAny.hpp"
#include "StarDynamicArray.hpp"
#include "StarDataStream.hpp"
#include "StarStepInterpolation.hpp"

namespace Star {

STAR_EXCEPTION(NetStatesException, StarException);

STAR_CLASS(NetStepStates);

// NetStates values that can be interpolated can be interpolated using the following methods:
//
// - "exact" interpolation, meaning that the interpolator simply does not look
//   into the future at all and does no interpolation, which is the default.
//   This is the same method that is used for values that cannot be
//   interpolated at all.
//
// - "lerp" interpolation, or simple linear interpolation.
//
// - A custom interpolation function, for values that where it may not make
//   sense to use linear interpolation, such as circular values.
typedef function<double(double, double, double)> NetStatesInterpolator;
NetStatesInterpolator const NetStatesExactInterpolator = NetStatesInterpolator();
NetStatesInterpolator const NetStatesLerpInterpolator = lerp<double, double>;
NetStatesInterpolator const NetStatesAngleInterpolator = angleLerp<double, double>;

namespace NetStatesDetail {
  typedef Any<int64_t, uint64_t, size_t, double, bool, shared_ptr<void>> Value;

  STAR_STRUCT(TypeInfo);
  STAR_CLASS(Field);

  struct GenericSerializer {
    virtual ~GenericSerializer() = default;

    virtual shared_ptr<void> read(DataStream& ds) const = 0;
    virtual void write(DataStream& ds, shared_ptr<void> const& data) const = 0;
  };
}

// A handle to a single field in a NetStates object.  Fields are designed to be
// very fast to set and get, so they can generally be used in place of a
// regular variable without much cost.
class NetStatesField {
public:
  NetStatesField(NetStatesField const&) = delete;
  NetStatesField(NetStatesField&&) = default;

  NetStatesField& operator=(NetStatesField const&) = delete;
  NetStatesField& operator=(NetStatesField&&) = default;

  // Pull whether or not the field has been updated since the last call to
  // pullUpdated.
  bool pullUpdated() const;

  // Is this field connected to a NetStepStates object?
  bool connected() const;

protected:
  friend NetStepStates;

  NetStatesField(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value);

  NetStatesDetail::FieldPtr m_field;
};

// NetStates integers are clamped BOTH when setting and when receiving delta
// updates to the available bit width, 8, 16, or 64 bit.
class NetStatesInt : public NetStatesField {
public:
  static NetStatesInt makeInt8(int64_t initialValue = 0);
  static NetStatesInt makeInt16(int64_t initialValue = 0);
  static NetStatesInt makeVarInt(int64_t initialValue = 0);

  NetStatesInt(NetStatesInt&&) = default;
  NetStatesInt& operator=(NetStatesInt&&) = default;

  int64_t get() const;
  void set(int64_t value);

private:
  NetStatesInt(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value);
};

class NetStatesUInt : public NetStatesField {
public:
  static NetStatesUInt makeUInt8(uint64_t initialValue = 0);
  static NetStatesUInt makeUInt16(uint64_t initialValue = 0);
  static NetStatesUInt makeVarUInt(uint64_t initialValue = 0);

  NetStatesUInt(NetStatesUInt&&) = default;
  NetStatesUInt& operator=(NetStatesUInt&&) = default;

  uint64_t get() const;
  void set(uint64_t value);

private:
  NetStatesUInt(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value);
};

// Properly encodes NPos no matter the platform width of size_t NetStates
// size_t values are NOT clamped when setting.
class NetStatesSize : public NetStatesField {
public:
  NetStatesSize(size_t initialValue = 0);

  NetStatesSize(NetStatesSize&&) = default;
  NetStatesSize& operator=(NetStatesSize&&) = default;

  size_t get() const;
  void set(size_t value);
};

// NetStates floats are clamped to their maximum and minimum legal values when
// calling set if the type is one of the normalized float types NFloat8 or
// NFloat16, or one of the fixed point types Fixed8 or Fixed16 (but not
// VarFixed)
class NetStatesFloat : public NetStatesField {
public:
  static NetStatesFloat makeFloat(double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  static NetStatesFloat makeDouble(double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  // Float only from 0.0 to 1.0
  static NetStatesFloat makeNormalizedFloat8(double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  static NetStatesFloat makeNormalizedFloat16(double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  // Float represented as a rational number of a fixed base
  static NetStatesFloat makeFixedPoint8(double base, double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  static NetStatesFloat makeFixedPoint16(double base, double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);
  static NetStatesFloat makeFixedPoint(double base, double initialValue = 0.0, NetStatesInterpolator interpolator = NetStatesExactInterpolator);

  NetStatesFloat(NetStatesFloat&&) = default;
  NetStatesFloat& operator=(NetStatesFloat&&) = default;

  double get() const;
  void set(double value);

  void setInterpolator(NetStatesInterpolator interpolator);

private:
  NetStatesFloat(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value);
};

class NetStatesBool : public NetStatesField {
public:
  NetStatesBool(bool initialValue = false);

  NetStatesBool(NetStatesBool&&) = default;
  NetStatesBool& operator=(NetStatesBool&&) = default;

  bool get() const;
  void set(bool value);
};

// Wraps a uint64_t to give a simple event stream.  Every trigger is an
// increment to a held uint64_t value, and slaves can see how many triggers
// have occurred since the last check.
class NetStatesEvent : public NetStatesField {
public:
  NetStatesEvent();

  NetStatesEvent(NetStatesEvent&&) = default;
  NetStatesEvent& operator=(NetStatesEvent&&) = default;

  void trigger();

  // Returns the number of times this event has been triggered since the last
  // pullOccurrences call.
  uint64_t pullOccurrences();

  // Pulls whether this event occurred at all, ignoring the number
  bool pullOccurred();

  // Ignore all the existing ocurrences
  void ignoreOccurrences();

private:
  using NetStatesField::pullUpdated;
};

// Holds an arbitrary serializable value
template<typename T>
class NetStatesData : public NetStatesField {
public:
  NetStatesData(T initialValue = T());
  NetStatesData(function<void(DataStream&, T&)> reader, function<void(DataStream&, T const&)> writer, T initialValue = T());

  NetStatesData(NetStatesData&&) = default;
  NetStatesData& operator=(NetStatesData&&) = default;

  T const& get() const;

  // Updates the value if the value is different than the existing value,
  // requires T have operator==
  void set(T const& value);

  // Always updates the value and marks it as updated.
  void push(T value);

  // Update the value in place.  The mutator will be called as bool
  // mutator(T&), return true to signal that the value was updated.
  template<typename Mutator>
  void update(Mutator&& mutator);

private:
  struct SerializerImpl : NetStatesDetail::GenericSerializer {
    SerializerImpl(function<void(DataStream&, T&)> reader, function<void(DataStream&, T const&)> writer);

    function<void(DataStream&, T&)> reader;
    function<void(DataStream&, T const&)> writer;

    shared_ptr<void> read(DataStream& ds) const override;
    void write(DataStream& ds, shared_ptr<void> const& data) const override;
  };
};

// Convenience around NetStatesInt to hold an enum value (avoids casting)
template<typename T>
class NetStatesEnum : public NetStatesField {
public:
  NetStatesEnum(T initialValue = T());

  T get() const;
  void set(T value);
};

// Convenience for very common NetStatesData types
typedef NetStatesData<String> NetStatesString;
typedef NetStatesData<ByteArray> NetStatesBytes;

// Send and receives state using delta coding based on a always increasing step
// value.  The step is never sent over the wire, rather it is used as a way to
// track versions of states.
class NetStepStates {
public:
  NetStepStates();
  template<typename... Args>
  NetStepStates(Args const&... fields);

  NetStepStates(NetStepStates const& states) = delete;
  NetStepStates& operator=(NetStepStates const& states) = delete;

  // Add the given unconnected field and connect it to this NetStepStates
  // instance.  Individual fields in a NetStates object are identified based on
  // the order in which they are added.  Both sides of a NetStates object must
  // add the same number of fields and of the same type, and in the same order,
  // for communication to work.
  void addField(NetStatesField const& nsfield);

  template<typename... Args>
  void addFields(Args const&... fields);

  // Removes all fields and resets back to the initial state.
  void clearFields();

  // Produces a digest of all the field types in this NetStates object
  uint32_t fieldDigest() const;

  // When sending, it is important *when* precisely updateStep is called in
  // order not to lose data, the order *must* be:
  // - updateStep(<increased step value>)
  // - *do things that change data*
  // - *write deltas*
  // - repeat.
  // No data should be set in between writing a delta and incrementing step, or
  // data will be lost to slaves.  (Assuming no delta overlapping is done to
  // prevent it).
  //
  // An exception will be thrown if newStep ever decreases in a call to
  // updateStep, reset must be called instead.
  //
  // Returns true in the event that updating the step changed the value of any
  // fields.  This will only be true when interpolation is enabled.
  bool updateStep(uint64_t newStep);
  uint64_t currentStep() const;

  // Start the NetStepStates object back from the initial step (will all the
  // existing fields).  All current field values are kept unchanged and
  // interpolation data is cleared.
  void resetStep();

  // Enables interpolation mode.  If interpolation mode is enabled, then all
  // incoming states are marked with a floating target step value, and states
  // are delayed until the interpolation step reaches the given step value for
  // that state.  This also enables optional interpolation smoothing on
  // floating point values that looks ahead into the future to even out
  // changes, and can also optionally extrapolate into the future if no data is
  // available.
  //
  // While interpolating, calling a set will discard any held interpolation
  // data even into the future.
  void enableInterpolation(uint64_t extrapolationSteps = 0);
  void disableInterpolation();
  bool interpolationEnabled() const;

  // Returns true if there is changed data since this timestep, and writeDelta
  // called with this step would write data.
  bool hasDelta(uint64_t fromStep) const;

  // Write all the state changes that have happened since (and including)
  // fromStep.  The normal way to use this would be each call to writeDelta
  // should be passed the step at the time of the *last* call to writeDelta, +
  // 1.  writeDelta with fromStep = 0 writes all states, and is a larger
  // version of writeFull().  If hasDelta(fromStep) returns false, this will
  // still write some data as an end marker, it is best to call hasDelta first
  // to see if any delta is needed.
  void writeDelta(DataStream& ds, uint64_t fromStep) const;

  // predictedDeltaStep is only necessary if interpolation is enabled,
  // otherwise the delta is assumed to apply immediately.  Will return true
  // when reading this delta has changed the current state of any fields.
  bool readDelta(DataStream& ds, double predictedDeltaStep = 0);

  // Writes / reads all values regardless of changed state.  Usually a slight
  // network improvement over writeDelta(0), as it does not need to prefix
  // every field with an index.  Additionall, first writes the 4 byte field
  // digest to the full state, so readFull also acts as a type consistency
  // check.
  void writeFull(DataStream& ds) const;
  void readFull(DataStream& ds, double predictedDeltaStep = 0);

  // Write and read deltas directly to a byte array.  Uses a slightly different
  // binary format that relies on the size of the array packet, so cannot be
  // mixed with normal writes and reads.
  ByteArray writeDeltaPacket(uint64_t fromStep) const;
  bool readDeltaPacket(ByteArray packet, double predictedDeltaStep = 0);
  ByteArray writeFullPacket() const;
  void readFullPacket(ByteArray packet, double predictedDeltaStep = 0);

  // Equivalent to reading a blank delta state, simply lets receiver know that
  // no delta is needed for up to this timestep, so no extrapolation is
  // required.  Not required if interpolation is not enabled or is enabled with
  // zero extrapolation steps.
  void interpolationHeartbeat(double predictedDeltaStep = 0);

private:
  DynamicArray<NetStatesDetail::FieldPtr> m_fields;
  uint64_t m_currentStep;
  bool m_interpolationEnabled;
  uint64_t m_extrapolationSteps;
};

// NetStepStates with no concept of a step, only sends values that have been
// changed between writes.  Only works for writing to a single receiver.
class NetSyncStates : private NetStepStates {
public:
  NetSyncStates();
  template<typename... Args>
  NetSyncStates(Args const&... fields);

  bool hasDelta() const;
  void writeDelta(DataStream& ds);
  bool readDelta(DataStream& ds);

  ByteArray writeDeltaPacket();
  bool readDeltaPacket(ByteArray packet);

  void reset();

  using NetStepStates::addField;
  using NetStepStates::clearFields;
  using NetStepStates::fieldDigest;
};

namespace NetStatesDetail {
  // Values as they are sent across the wire, converted to their compressed or
  // truncated transmission format.
  typedef Any<int8_t, uint8_t, int16_t, uint16_t, int64_t, uint64_t, float, double, bool, shared_ptr<void>> TransmissionValue;

  enum class TransmissionType : uint8_t {
    Int8,
    UInt8,
    Int16,
    UInt16,
    VarInt,
    VarUInt,
    Size,
    Float,
    Double,
    NFloat8,
    NFloat16,
    Fixed8,
    Fixed16,
    VarFixed,
    Bool,
    Event,
    Generic
  };

  struct TypeInfo {
    TransmissionType type;

    // If this is a fixed point float, this is its fixed point base
    Maybe<double> fixedPointBase;

    // If the type is Data, this holds the reader / writer for it.
    shared_ptr<GenericSerializer> genericSerializer;

    // If this is any sort of floating type, then if value interpolation is
    // enabled this will hold the interpolator
    NetStatesInterpolator interpolator;

    Value valueFromTransmission(TransmissionValue const& transmission) const;
    TransmissionValue transmissionFromValue(Value const& value) const;
  };

  class Field {
  public:
    Field(TypeInfo typeInfo, Value value);

    TypeInfo const& typeInfo() const;

    int64_t getInt() const;
    void setInt(int64_t value);

    uint64_t getUInt() const;
    void setUInt(uint64_t value);

    size_t getSize() const;
    void setSize(size_t value);

    double getFloat() const;
    void setFloat(double value);
    void setFloatInterpolator(NetStatesInterpolator interpolator);

    bool getBool() const;
    void setBool(bool value);

    void triggerEvent();
    uint64_t pullOccurrences();
    void ignoreOccurrences();

    template<typename T>
    T const& getGeneric() const;
    template<typename T>
    void setGeneric(T const& data);
    template<typename T>
    void pushGeneric(T data);
    template<typename T, typename Mutator>
    void updateGeneric(Mutator&& mutator);

    bool pullUpdated();

    // Returns true if field value may have changed
    bool updateStep(uint64_t step);
    void resetStep();

    bool hasDelta(uint64_t fromStep) const;
    bool readField(DataStream& ds, double interpolationStep);
    void writeField(DataStream& ds) const;

    void enableInterpolation(uint64_t extrapolationSteps);
    void disableInterpolation();
    void interpolationHeartbeat(double predictedDeltaStep);

  private:
    bool updateValueFromTransmission();

    // Marks the value as updated by setting the unpulledUpdate flag and the
    // transmissionDirty flag
    void markValueUpdated();

    // To support fast setting of values, transmission values are updated
    // lazily.  Once the transmissionDirty flag is set, call
    // freshenTransmission() to (IF the transmission value has changed) update
    // the latest transmission and latest transmission step from the current
    // value.
    void freshenTransmission();

    TypeInfo m_typeInfo;
    uint64_t m_currentStep;
    Value m_value;

    bool m_unpulledUpdate;
    uint64_t m_pulledOccurrences;

    bool m_transmissionDirty;
    TransmissionValue m_latestTransmission;
    uint64_t m_latestTransmissionStep;
    Maybe<StepStream<TransmissionValue>> m_interpolatedTransmissions;
  };
}

template<typename T>
NetStatesData<T>::NetStatesData(T initialValue)
  : NetStatesData(
    [](DataStream& ds, T& t) {
      ds >> t;
    },
    [](DataStream& ds, T const& t) {
      ds << t;
    }, move(initialValue)) {}

template<typename T>
NetStatesData<T>::NetStatesData(function<void(DataStream&, T&)> reader, function<void(DataStream&, T const&)> writer, T initialValue)
  : NetStatesField(NetStatesDetail::TypeInfo{
        NetStatesDetail::TransmissionType::Generic, {},
        make_shared<SerializerImpl>(move(reader), move(writer)), {}
      }, shared_ptr<void>(make_shared<T>(move(initialValue)))) {}

template<typename T>
T const& NetStatesData<T>::get() const {
  return m_field->template getGeneric<T>();
}

template<typename T>
void NetStatesData<T>::set(T const& value) {
  m_field->template setGeneric<T>(value);
}

template<typename T>
void NetStatesData<T>::push(T value) {
  m_field->template pushGeneric<T>(move(value));
}

template<typename T>
template<typename Mutator>
void NetStatesData<T>::update(Mutator&& mutator) {
  m_field->template updateGeneric<T>(forward<Mutator>(mutator));
}

template<typename T>
NetStatesData<T>::SerializerImpl::SerializerImpl(function<void(DataStream&, T&)> reader, function<void(DataStream&, T const&)> writer)
  : reader(move(reader)), writer(move(writer)) {}

template<typename T>
shared_ptr<void> NetStatesData<T>::SerializerImpl::read(DataStream& ds) const {
  auto data = make_shared<T>();
  reader(ds, *data);
  return data;
}

template<typename T>
void NetStatesData<T>::SerializerImpl::write(DataStream& ds, shared_ptr<void> const& data) const {
  writer(ds, *(T*)data.get());
}

template<typename T>
NetStatesEnum<T>::NetStatesEnum(T initialValue)
  : NetStatesField(
      NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::VarInt, {}, {}, {}},
      (int64_t)initialValue) {}

template<typename T>
T NetStatesEnum<T>::get() const {
  return (T)m_field->getInt();
}

template<typename T>
void NetStatesEnum<T>::set(T value) {
  return m_field->setInt((int64_t)value);
}

template<typename... Args>
NetStepStates::NetStepStates(Args const&... fields)
  : NetStepStates() {
  addFields(fields...);
}

template<typename... Args>
void NetStepStates::addFields(Args const&... fields) {
  callFunctionVariadic([this](NetStatesField const& field) {
      addField(field);
    }, fields...);
}

template<typename... Args>
NetSyncStates::NetSyncStates(Args const&... fields)
  : NetSyncStates() {
  addFields(fields...);
}

template<typename T>
T const& NetStatesDetail::Field::getGeneric() const {
  starAssert(m_value.get<shared_ptr<void>>().get() != nullptr);
  return *(T const*)m_value.get<shared_ptr<void>>().get();
}

template<typename T>
void NetStatesDetail::Field::setGeneric(T const& data) {
  auto& p = m_value.get<shared_ptr<void>>();
  if (!(*(T*)p.get() == data)) {
    *(T*)p.get() = data;
    markValueUpdated();
  }
}

template<typename T>
void NetStatesDetail::Field::pushGeneric(T data) {
  if (m_value) {
    auto& p = m_value.get<shared_ptr<void>>();
    *(T*)p.get() = move(data);
  } else {
    m_value = shared_ptr<void>(make_shared<T>(move(data)));
  }
  markValueUpdated();
}

template<typename T, typename Mutator>
void NetStatesDetail::Field::updateGeneric(Mutator&& mutator) {
  auto const& p = m_value.get<shared_ptr<void>>();
  if (mutator(*(T*)p.get()))
    markValueUpdated();
}

}

#endif
