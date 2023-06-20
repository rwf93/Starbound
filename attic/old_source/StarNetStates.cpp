#include "StarNetStates.hpp"

namespace Star {

bool NetStatesField::pullUpdated() const {
  return m_field->pullUpdated();
}

bool NetStatesField::connected() const {
  return !m_field.unique();
}

NetStatesField::NetStatesField(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value)
  : m_field(make_shared<NetStatesDetail::Field>(move(typeInfo), move(value))) {}

NetStatesInt NetStatesInt::makeInt8(int64_t initialValue) {
  return NetStatesInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Int8, {}, {}, {}}, initialValue);
}

NetStatesInt NetStatesInt::makeInt16(int64_t initialValue) {
  return NetStatesInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Int16, {}, {}, {}}, initialValue);
}

NetStatesInt NetStatesInt::makeVarInt(int64_t initialValue) {
  return NetStatesInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::VarInt, {}, {}, {}}, initialValue);
}

NetStatesInt::NetStatesInt(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value)
  : NetStatesField(move(typeInfo), move(value)) {}

int64_t NetStatesInt::get() const {
  return m_field->getInt();
}

void NetStatesInt::set(int64_t value) {
  m_field->setInt(value);
}

NetStatesUInt NetStatesUInt::makeUInt8(uint64_t initialValue) {
  return NetStatesUInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::UInt8, {}, {}, {}}, initialValue);
}

NetStatesUInt NetStatesUInt::makeUInt16(uint64_t initialValue) {
  return NetStatesUInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::UInt16, {}, {}, {}}, initialValue);
}

NetStatesUInt NetStatesUInt::makeVarUInt(uint64_t initialValue) {
  return NetStatesUInt(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::VarUInt, {}, {}, {}}, initialValue);
}

NetStatesUInt::NetStatesUInt(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value)
  : NetStatesField(move(typeInfo), move(value)) {}

uint64_t NetStatesUInt::get() const {
  return m_field->getUInt();
}

void NetStatesUInt::set(uint64_t value) {
  m_field->setUInt(value);
}

NetStatesSize::NetStatesSize(size_t initialValue)
  : NetStatesField(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Size, {}, {}, {}}, initialValue) {}

size_t NetStatesSize::get() const {
  return m_field->getSize();
}

void NetStatesSize::set(size_t value) {
  m_field->setSize(value);
}

NetStatesFloat NetStatesFloat::makeFloat(double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Float, {}, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeDouble(double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Double, {}, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeNormalizedFloat8(double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::NFloat8, {}, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeNormalizedFloat16(double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::NFloat16, {}, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeFixedPoint8(double base, double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Fixed8, base, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeFixedPoint16(double base, double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Fixed16, base, {}, move(interpolator)}, initialValue);
}

NetStatesFloat NetStatesFloat::makeFixedPoint(double base, double initialValue, NetStatesInterpolator interpolator) {
  return NetStatesFloat(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::VarFixed, base, {}, move(interpolator)}, initialValue);
}

double NetStatesFloat::get() const {
  return m_field->getFloat();
}

void NetStatesFloat::set(double value) {
  m_field->setFloat(value);
}

void NetStatesFloat::setInterpolator(NetStatesInterpolator interpolator) {
  m_field->setFloatInterpolator(move(interpolator));
}

NetStatesFloat::NetStatesFloat(NetStatesDetail::TypeInfo typeInfo, NetStatesDetail::Value value)
  : NetStatesField(move(typeInfo), move(value)) {}

NetStatesBool::NetStatesBool(bool initialValue)
  : NetStatesField(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Bool, {}, {}, {}}, initialValue) {}

bool NetStatesBool::get() const {
  return m_field->getBool();
}

void NetStatesBool::set(bool value) {
  m_field->setBool(value);
}

NetStatesEvent::NetStatesEvent()
  : NetStatesField(NetStatesDetail::TypeInfo{NetStatesDetail::TransmissionType::Event, {}, {}, {}}, (uint64_t)0) {}

void NetStatesEvent::trigger() {
  m_field->triggerEvent();
}

uint64_t NetStatesEvent::pullOccurrences() {
  return m_field->pullOccurrences();
}

bool NetStatesEvent::pullOccurred() {
  return pullOccurrences() != 0;
}

void NetStatesEvent::ignoreOccurrences() {
  m_field->ignoreOccurrences();
}

NetStepStates::NetStepStates() {
  clearFields();
}

void NetStepStates::addField(NetStatesField const& nsfield) {
  if (nsfield.connected())
    throw NetStatesException("Field added in NetStepStates is already connected");

  auto field = nsfield.m_field;
  field->updateStep(m_currentStep);
  if (m_interpolationEnabled)
    field->enableInterpolation(m_extrapolationSteps);
  m_fields.append(field);
}

void NetStepStates::clearFields() {
  for (auto const& field : m_fields) {
    field->disableInterpolation();
    field->resetStep();
  }

  m_fields.clear();
  m_currentStep = 0;
  m_interpolationEnabled = false;
  m_extrapolationSteps = 0;
}

uint32_t NetStepStates::fieldDigest() const {
  uint32_t digest;
  fnv_hash32_init(digest);
  for (auto const& field : m_fields) {
    auto const& typeInfo = field->typeInfo();
    uint8_t type = (uint8_t)typeInfo.type;
    float base = typeInfo.fixedPointBase.value(0.0f);
    toBigEndian(type);
    toBigEndian(base);
    fnv_hash32_bytes(digest, (char const*)&type, sizeof(type));
    fnv_hash32_bytes(digest, (char const*)&base, sizeof(base));
  }
  return digest;
}

bool NetStepStates::updateStep(uint64_t step) {
  if (step < m_currentStep) {
    throw NetStatesException("step decreased in NetStepStates::updateStep");
  } else if (step > m_currentStep) {
    m_currentStep = step;
    bool updated = false;
    for (auto const& field : m_fields)
      updated |= field->updateStep(m_currentStep);
    return updated;
  } else {
    return false;
  }
}

uint64_t NetStepStates::currentStep() const {
  return m_currentStep;
}

void NetStepStates::resetStep() {
  m_currentStep = 0;
  for (auto& field : m_fields)
    field->resetStep();
}

void NetStepStates::enableInterpolation(uint64_t extrapolationSteps) {
  m_interpolationEnabled = true;
  m_extrapolationSteps = extrapolationSteps;
  for (auto& field : m_fields)
    field->enableInterpolation(extrapolationSteps);
}

void NetStepStates::disableInterpolation() {
  m_interpolationEnabled = false;
  m_extrapolationSteps = 0.0;
  for (auto& field : m_fields)
    field->disableInterpolation();
}

bool NetStepStates::interpolationEnabled() const {
  return m_interpolationEnabled;
}

bool NetStepStates::hasDelta(uint64_t fromStep) const {
  for (auto const& field : m_fields) {
    if (field->hasDelta(fromStep))
      return true;
  }
  return false;
}

void NetStepStates::writeDelta(DataStream& ds, uint64_t fromStep) const {
  for (size_t i = 0; i < m_fields.size(); ++i) {
    auto const& field = m_fields[i];
    if (field->hasDelta(fromStep)) {
      ds.writeVlqU(i + 1);
      field->writeField(ds);
    }
  }
  ds.writeVlqU(0);
}

bool NetStepStates::readDelta(DataStream& ds, double predictedDeltaStep) {
  bool updated = false;
  while (true) {
    uint64_t index = ds.readVlqU();
    if (index == 0)
      break;
    --index;
    updated |= m_fields.at(index)->readField(ds, predictedDeltaStep);
  }

  interpolationHeartbeat(predictedDeltaStep);
  return updated;
}

void NetStepStates::writeFull(DataStream& ds) const {
  ds.write(fieldDigest());
  for (auto const& field : m_fields)
    field->writeField(ds);
}

void NetStepStates::readFull(DataStream& ds, double predictedDeltaStep) {
  auto digest = ds.read<uint32_t>();
  if (digest != fieldDigest())
    throw NetStatesException("Type mismatch in NetStepStates::readFull");

  for (auto const& field : m_fields)
    field->readField(ds, predictedDeltaStep);
}

ByteArray NetStepStates::writeDeltaPacket(uint64_t fromStep) const {
  DataStreamBuffer ds;

  for (size_t i = 0; i < m_fields.size(); ++i) {
    auto const& field = m_fields[i];
    if (field->hasDelta(fromStep)) {
      ds.writeVlqU(i);
      field->writeField(ds);
    }
  }

  return ds.takeData();
}

bool NetStepStates::readDeltaPacket(ByteArray packet, double predictedDeltaStep) {
  DataStreamBuffer ds(move(packet));

  bool updated = false;
  while (!ds.atEnd()) {
    uint64_t index = ds.readVlqU();
    updated |= m_fields.at(index)->readField(ds, predictedDeltaStep);
  }

  interpolationHeartbeat(predictedDeltaStep);
  return updated;
}

ByteArray NetStepStates::writeFullPacket() const {
  DataStreamBuffer ds;
  writeFull(ds);
  return ds.takeData();
}

void NetStepStates::readFullPacket(ByteArray packet, double predictedDeltaStep) {
  DataStreamBuffer ds(move(packet));
  readFull(ds, predictedDeltaStep);
}

void NetStepStates::interpolationHeartbeat(double predictedDeltaStep) {
  if (m_interpolationEnabled) {
    for (auto const& field : m_fields)
      field->interpolationHeartbeat(predictedDeltaStep);
  }
}

NetSyncStates::NetSyncStates() {}

bool NetSyncStates::hasDelta() const {
  return NetStepStates::hasDelta(currentStep());
}

void NetSyncStates::writeDelta(DataStream& ds) {
  NetStepStates::writeDelta(ds, currentStep());
  updateStep(currentStep() + 1);
}

bool NetSyncStates::readDelta(DataStream& ds) {
  updateStep(currentStep() + 1);
  return NetStepStates::readDelta(ds, currentStep());
}

ByteArray NetSyncStates::writeDeltaPacket() {
  auto packet = NetStepStates::writeDeltaPacket(currentStep());
  updateStep(currentStep() + 1);
  return packet;
}

bool NetSyncStates::readDeltaPacket(ByteArray packet) {
  updateStep(currentStep() + 1);
  return NetStepStates::readDeltaPacket(move(packet), currentStep());
}

void NetSyncStates::reset() {
  NetStepStates::resetStep();
}

NetStatesDetail::Value NetStatesDetail::TypeInfo::valueFromTransmission(NetStatesDetail::TransmissionValue const& transmission) const {
  if (type == TransmissionType::Int8) {
    return (int64_t)transmission.get<int8_t>();
  } else if (type == TransmissionType::UInt8) {
    return (uint64_t)transmission.get<uint8_t>();
  } else if (type == TransmissionType::Int16) {
    return (int64_t)transmission.get<int16_t>();
  } else if (type == TransmissionType::UInt16) {
    return (uint64_t)transmission.get<uint16_t>();
  } else if (type == TransmissionType::VarInt) {
    return (int64_t)transmission.get<int64_t>();
  } else if (type == TransmissionType::VarUInt) {
    return (uint64_t)transmission.get<uint64_t>();
  } else if (type == TransmissionType::Size) {
    uint64_t u = transmission.get<uint64_t>();
    if (u == 0)
      return NPos;
    return (size_t)(u - 1);
  } else if (type == TransmissionType::Float) {
    return (double)transmission.get<float>();
  } else if (type == TransmissionType::Double) {
    return transmission.get<double>();
  } else if (type == TransmissionType::NFloat8) {
    return transmission.get<uint8_t>() / 255.0;
  } else if (type == TransmissionType::NFloat16) {
    return transmission.get<uint16_t>() / 65535.0;
  } if (type == TransmissionType::Fixed8) {
    return transmission.get<int8_t>() * *fixedPointBase;
  } else if (type == TransmissionType::Fixed16) {
    return transmission.get<int16_t>() * *fixedPointBase;
  } else if (type == TransmissionType::VarFixed) {
    return transmission.get<int64_t>() * *fixedPointBase;
  } else if (type == TransmissionType::Bool) {
    return transmission.get<bool>();
  } else if (type == TransmissionType::Event) {
    return transmission.get<uint64_t>();
  } else {
    return transmission.get<shared_ptr<void>>();
  }
}

NetStatesDetail::TransmissionValue NetStatesDetail::TypeInfo::transmissionFromValue(NetStatesDetail::Value const& value) const {
  if (type == TransmissionType::Int8) {
    return (int8_t)value.get<int64_t>();
  } else if (type == TransmissionType::UInt8) {
    return (uint8_t)value.get<uint64_t>();
  } else if (type == TransmissionType::Int16) {
    return (int16_t)value.get<int64_t>();
  } else if (type == TransmissionType::UInt16) {
    return (uint16_t)value.get<uint64_t>();
  } else if (type == TransmissionType::VarInt) {
    return (int64_t)value.get<int64_t>();
  } else if (type == TransmissionType::VarUInt) {
    return (uint64_t)value.get<uint64_t>();
  } else if (type == TransmissionType::Size) {
    size_t s = value.get<size_t>();
    if (s == NPos)
      return (uint64_t)0;
    return (uint64_t)(s + 1);
  } else if (type == TransmissionType::Float) {
    return (float)value.get<double>();
  } else if (type == TransmissionType::Double) {
    return (double)value.get<double>();
  } else if (type == TransmissionType::NFloat8) {
    return (uint8_t)round(clamp(value.get<double>(), 0.0, 1.0) * 255.0);
  } else if (type == TransmissionType::NFloat16) {
    return (uint16_t)round(clamp(value.get<double>(), 0.0, 1.0) * 65535.0);
  } else if (type == TransmissionType::Fixed8) {
    return (int8_t)clamp<int64_t>(round(value.get<double>() / *fixedPointBase), -128, 127);
  } else if (type == TransmissionType::Fixed16) {
    return (int16_t)clamp<int64_t>(round(value.get<double>() / *fixedPointBase), -32768, 32767);
  } else if (type == TransmissionType::VarFixed) {
    return (int64_t)round(value.get<double>() / *fixedPointBase);
  } else if (type == TransmissionType::Bool) {
    return value.get<bool>();
  } else if (type == TransmissionType::Event) {
    return value.get<uint64_t>();
  } else {
    return value.get<shared_ptr<void>>();
  }
}

NetStatesDetail::Field::Field(TypeInfo typeInfo, Value value)
  : m_typeInfo(move(typeInfo)), m_currentStep(0), m_value(move(value)),
    m_unpulledUpdate(true), m_pulledOccurrences(0), m_transmissionDirty(true),
    m_latestTransmissionStep(0) {
  freshenTransmission();
}

NetStatesDetail::TypeInfo const& NetStatesDetail::Field::typeInfo() const {
  return m_typeInfo;
}

int64_t NetStatesDetail::Field::getInt() const {
  return m_value.get<int64_t>();
}

void NetStatesDetail::Field::setInt(int64_t value) {
  if (m_typeInfo.type == TransmissionType::Int8)
    value = clamp<int64_t>(value, -128, 127);
  else if (m_typeInfo.type == TransmissionType::Int16)
    value = clamp<int64_t>(value, -32768, 32767);

  if (m_value != value) {
    m_value = value;
    markValueUpdated();
  }
}

uint64_t NetStatesDetail::Field::getUInt() const {
  return m_value.get<uint64_t>();
}

void NetStatesDetail::Field::setUInt(uint64_t value) {
  if (m_typeInfo.type == TransmissionType::UInt8)
    value = clamp<uint64_t>(value, 0, 255);
  else if (m_typeInfo.type == TransmissionType::UInt16)
    value = clamp<uint64_t>(value, 0, 65535);

  if (m_value != value) {
    m_value = value;
    markValueUpdated();
  }
}

size_t NetStatesDetail::Field::getSize() const {
  return m_value.get<size_t>();
}

void NetStatesDetail::Field::setSize(size_t value) {
  if (m_value != value) {
    m_value = value;
    markValueUpdated();
  }
}

double NetStatesDetail::Field::getFloat() const {
  return m_value.get<double>();
}

void NetStatesDetail::Field::setFloat(double value) {
  if (m_typeInfo.type == TransmissionType::NFloat8 || m_typeInfo.type == TransmissionType::NFloat16)
    value = clamp<double>(value, 0.0, 1.0);
  else if (m_typeInfo.type == TransmissionType::Fixed8)
    value = clamp<double>(value, -128.0 * *m_typeInfo.fixedPointBase, 127.0 * *m_typeInfo.fixedPointBase);
  else if (m_typeInfo.type == TransmissionType::Fixed16)
    value = clamp<double>(value, -32768.0 * *m_typeInfo.fixedPointBase, 32767.0 * *m_typeInfo.fixedPointBase);

  if (m_value != value) {
    m_value = value;
    markValueUpdated();
  }
}

void NetStatesDetail::Field::setFloatInterpolator(NetStatesInterpolator interpolator) {
  m_typeInfo.interpolator = move(interpolator);
}

bool NetStatesDetail::Field::getBool() const {
  return m_value.get<bool>();
}

void NetStatesDetail::Field::setBool(bool value) {
  if (m_value != value) {
    m_value = value;
    markValueUpdated();
  }
}

void NetStatesDetail::Field::triggerEvent() {
  ++m_value.get<uint64_t>();
  markValueUpdated();
}

uint64_t NetStatesDetail::Field::pullOccurrences() {
  uint64_t occurrences = m_value.get<uint64_t>();
  starAssert(occurrences >= m_pulledOccurrences);
  uint64_t unchecked = occurrences - m_pulledOccurrences;
  m_pulledOccurrences = occurrences;
  return unchecked;
}

void NetStatesDetail::Field::ignoreOccurrences() {
  m_pulledOccurrences = m_value.get<uint64_t>();
}

bool NetStatesDetail::Field::pullUpdated() {
  return take(m_unpulledUpdate);
}

bool NetStatesDetail::Field::updateStep(uint64_t step) {
  if (m_interpolatedTransmissions) {
    freshenTransmission();
    if (m_interpolatedTransmissions->updateStep(step))
      return updateValueFromTransmission();
  }
  m_currentStep = step;
  return false;
}

void NetStatesDetail::Field::resetStep() {
  freshenTransmission();
  m_currentStep = 0;
  m_latestTransmissionStep = 0;

  if (m_interpolatedTransmissions) {
    m_interpolatedTransmissions->reset();
    m_interpolatedTransmissions->appendDataPoint(m_currentStep, m_latestTransmission);
  }
}

bool NetStatesDetail::Field::hasDelta(uint64_t fromStep) const {
  const_cast<Field*>(this)->freshenTransmission();
  return m_latestTransmissionStep >= fromStep;
}

bool NetStatesDetail::Field::readField(DataStream& ds, double interpolationStep) {
  freshenTransmission();

  if (m_typeInfo.type == TransmissionType::Int8 || m_typeInfo.type == TransmissionType::Fixed8) {
    ds.read(m_latestTransmission.get<int8_t>());
  } else if (m_typeInfo.type == TransmissionType::UInt8 || m_typeInfo.type == TransmissionType::NFloat8) {
    ds.read(m_latestTransmission.get<uint8_t>());
  } else if (m_typeInfo.type == TransmissionType::Int16 || m_typeInfo.type == TransmissionType::Fixed16) {
    ds.read(m_latestTransmission.get<int16_t>());
  } else if (m_typeInfo.type == TransmissionType::UInt16 || m_typeInfo.type == TransmissionType::NFloat16) {
    ds.read(m_latestTransmission.get<uint16_t>());
  } else if (m_typeInfo.type == TransmissionType::VarInt || m_typeInfo.type == TransmissionType::VarFixed) {
    m_latestTransmission.get<int64_t>() = ds.readVlqI();
  } else if (m_typeInfo.type == TransmissionType::VarUInt || m_typeInfo.type == TransmissionType::Size || m_typeInfo.type == TransmissionType::Event) {
    m_latestTransmission.get<uint64_t>() = ds.readVlqU();
  } else if (m_typeInfo.type == TransmissionType::Float) {
    ds.read(m_latestTransmission.get<float>());
  } else if (m_typeInfo.type == TransmissionType::Double) {
    ds.read(m_latestTransmission.get<double>());
  } else if (m_typeInfo.type == TransmissionType::Bool) {
    ds.read(m_latestTransmission.get<bool>());
  } else if (m_typeInfo.type == TransmissionType::Generic) {
    m_latestTransmission = shared_ptr<void>(m_typeInfo.genericSerializer->read(ds));
  }

  m_latestTransmissionStep = m_currentStep;

  bool updated = true;
  if (m_interpolatedTransmissions)
    updated = m_interpolatedTransmissions->appendDataPoint(interpolationStep, m_latestTransmission);

  if (updated)
    updateValueFromTransmission();

  return updated;
}

void NetStatesDetail::Field::writeField(DataStream& ds) const {
  const_cast<Field*>(this)->freshenTransmission();

  if (m_typeInfo.type == TransmissionType::Int8 || m_typeInfo.type == TransmissionType::Fixed8) {
    ds.write(m_latestTransmission.get<int8_t>());
  } else if (m_typeInfo.type == TransmissionType::UInt8 || m_typeInfo.type == TransmissionType::NFloat8) {
    ds.write(m_latestTransmission.get<uint8_t>());
  } else if (m_typeInfo.type == TransmissionType::Int16 || m_typeInfo.type == TransmissionType::Fixed16) {
    ds.write(m_latestTransmission.get<int16_t>());
  } else if (m_typeInfo.type == TransmissionType::UInt16 || m_typeInfo.type == TransmissionType::NFloat16) {
    ds.write(m_latestTransmission.get<uint16_t>());
  } else if (m_typeInfo.type == TransmissionType::VarInt || m_typeInfo.type == TransmissionType::VarFixed) {
    ds.writeVlqI(m_latestTransmission.get<int64_t>());
  } else if (m_typeInfo.type == TransmissionType::VarUInt || m_typeInfo.type == TransmissionType::Size || m_typeInfo.type == TransmissionType::Event) {
    ds.writeVlqU(m_latestTransmission.get<uint64_t>());
  } else if (m_typeInfo.type == TransmissionType::Float) {
    ds.write(m_latestTransmission.get<float>());
  } else if (m_typeInfo.type == TransmissionType::Double) {
    ds.write(m_latestTransmission.get<double>());
  } else if (m_typeInfo.type == TransmissionType::Bool) {
    ds.write(m_latestTransmission.get<bool>());
  } else if (m_typeInfo.type == TransmissionType::Generic) {
    m_typeInfo.genericSerializer->write(ds, m_latestTransmission.get<shared_ptr<void>>());
  }
}

void NetStatesDetail::Field::enableInterpolation(uint64_t extrapolationSteps) {
  freshenTransmission();
  m_interpolatedTransmissions.emplace();
  m_interpolatedTransmissions->setExtrapolation(extrapolationSteps);
  m_interpolatedTransmissions->appendDataPoint(m_currentStep, m_latestTransmission);
  m_interpolatedTransmissions->updateStep(m_latestTransmissionStep);
  m_interpolatedTransmissions->heartbeat(m_currentStep);
}

void NetStatesDetail::Field::disableInterpolation() {
  freshenTransmission();
  m_interpolatedTransmissions.reset();
}

void NetStatesDetail::Field::interpolationHeartbeat(double predictedDeltaStep) {
  freshenTransmission();
  if (m_interpolatedTransmissions)
    m_interpolatedTransmissions->heartbeat(predictedDeltaStep);
}

bool NetStatesDetail::Field::updateValueFromTransmission() {
  freshenTransmission();

  Value newValue;
  TransmissionValue storeExactValue;
  if (m_interpolatedTransmissions) {
    if (m_typeInfo.interpolator &&
        (m_typeInfo.type == TransmissionType::Float || m_typeInfo.type == TransmissionType::Double ||
        m_typeInfo.type == TransmissionType::NFloat8 || m_typeInfo.type == TransmissionType::NFloat16 ||
        m_typeInfo.type == TransmissionType::Fixed8 || m_typeInfo.type == TransmissionType::Fixed16 ||
        m_typeInfo.type == TransmissionType::VarFixed)) {
      auto weighting = m_interpolatedTransmissions->weighting();
      double min = m_typeInfo.valueFromTransmission(weighting.pointMin).get<double>();
      double max = m_typeInfo.valueFromTransmission(weighting.pointMax).get<double>();
      newValue = m_typeInfo.interpolator(weighting.offset, min, max);
    } else {
      newValue = m_typeInfo.valueFromTransmission(m_interpolatedTransmissions->exact());
    }
  } else {
    newValue = m_typeInfo.valueFromTransmission(m_latestTransmission);
  }

  if (m_value != newValue) {
    m_value = move(newValue);
    m_unpulledUpdate = true;
    return true;
  } else {
    return false;
  }
}

void NetStatesDetail::Field::markValueUpdated() {
  m_unpulledUpdate = true;
  m_transmissionDirty = true;
}

void NetStatesDetail::Field::freshenTransmission() {
  if (m_transmissionDirty) {
    auto newTransmission = m_typeInfo.transmissionFromValue(m_value);
    if (newTransmission.is<shared_ptr<void>>() || newTransmission != m_latestTransmission) {
      m_latestTransmissionStep = m_currentStep;
      m_latestTransmission = move(newTransmission);
    }

    if (m_interpolatedTransmissions) {
      m_interpolatedTransmissions->reset();
      m_interpolatedTransmissions->appendDataPoint(m_currentStep, m_latestTransmission);
    }
    m_transmissionDirty = false;
  }
}

}
