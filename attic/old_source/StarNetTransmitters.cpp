#include "StarNetTransmitters.hpp"
#include "StarIterator.hpp"

namespace Star {

NetVersionSender::NetVersionSender() {
  m_currentVersion = 0;
}

NetVersionSender::NetVersionSender(NetSchema const& schema)
  : NetVersionSender() {
  initialize(schema);
}

void NetVersionSender::initialize(NetSchema const& schema) {
  if (!schema.isFinalized())
    throw NetException("schema unfinalized in NetVersionSender::initialize");

  m_currentVersion = 0;

  m_valuesByName.clear();
  m_values.clear();
  m_schema = schema;

  for (auto id : m_schema.ids()) {
    ValuePtr value = std::make_shared<Value>();
    value->lastUpdated = m_currentVersion;

    // Initialize with default values for each NetType, so we can error if a
    // mismatched set was called.
    auto const& fieldInfo = m_schema.fieldInfo(id);
    if (fieldInfo.fixedPointBase != 0.0) {
      value->data.set<double>(0.0);
    } else if (fieldInfo.type == NetType::Event) {
      value->data.set<uint64_t>(0);
    } else if (netTypeIsSignedInt(fieldInfo.type)) {
      value->data.set<int64_t>(0);
    } else if (netTypeIsUnsignedInt(fieldInfo.type)) {
      value->data.set<uint64_t>(0);
    } else if (fieldInfo.type == NetType::Size) {
      value->data.set<size_t>(NPos);
    } else if (netTypeIsFloat(fieldInfo.type)) {
      value->data.set<double>(0.0);
    } else if (fieldInfo.type == NetType::Bool) {
      value->data.set<bool>(false);
    } else if (fieldInfo.type == NetType::String) {
      value->data.set<String>(String());
    } else if (fieldInfo.type == NetType::Data) {
      value->data.set<ByteArray>(ByteArray());
    } else if (fieldInfo.type == NetType::Variant) {
      value->data.set<Variant>(Variant());
    }

    m_values.insert(id, value);
    m_valuesByName.insert(m_schema.name(id), value);
  }
}

NetSchema const& NetVersionSender::schema() const {
  return m_schema;
}

void NetVersionSender::resetVersion() {
  m_currentVersion = 0;
  for (auto& pair : m_values)
    pair.second->lastUpdated = m_currentVersion;
}

void NetVersionSender::incrementVersion(uint64_t version) {
  if (version < m_currentVersion)
    throw NetException("Version reversed in NetVersionSender::incrementVersion");

  m_currentVersion = version;
}

uint64_t NetVersionSender::currentVersion() const {
  return m_currentVersion;
}

void NetVersionSender::triggerEvent(StringRef name) {
  auto const& value = getValue(name);
  if (!value->data.is<uint64_t>())
    throw NetException("NetSender type mismatch in triggerEvent");

  value->data.set<uint64_t>(value->data.get<uint64_t>() + 1);
  value->lastUpdated = m_currentVersion;
}

void NetVersionSender::setInt(StringRef name, int64_t v) {
  auto const& value = getValue(name);
  if (!value->data.is<int64_t>())
    throw NetException("NetSender type mismatch in setInt");

  if (value->data.cget<int64_t>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<int64_t>(v);
  }
}

void NetVersionSender::setUInt(StringRef name, uint64_t v) {
  auto const& value = getValue(name);
  if (!value->data.is<uint64_t>())
    throw NetException("NetSender type mismatch in setUInt");

  if (value->data.cget<uint64_t>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<uint64_t>(v);
  }
}

void NetVersionSender::setSize(StringRef name, size_t v) {
  auto const& value = getValue(name);
  if (!value->data.is<size_t>())
    throw NetException("NetSender type mismatch in setSize");

  if (value->data.cget<size_t>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<size_t>(v);
  }
}

void NetVersionSender::setFloat(StringRef name, double v) {
  auto const& value = getValue(name);
  if (!value->data.is<double>())
    throw NetException("NetSender type mismatch in setFloat");

  // TODO: For right now does a straight != comparison, should we include a
  // tolerance setting?
  if (value->data.cget<double>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<double>(v);
  }
}

void NetVersionSender::setBool(StringRef name, bool v) {
  auto const& value = getValue(name);
  if (!value->data.is<bool>())
    throw NetException("NetSender type mismatch in setBool");

  if (value->data.cget<bool>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<bool>(v);
  }
}

void NetVersionSender::setString(StringRef name, String const& v) {
  auto const& value = getValue(name);
  if (!value->data.is<String>())
    throw NetException("NetSender type mismatch in setString");

  if (value->data.cget<String>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<String>(v);
  }
}

void NetVersionSender::setData(StringRef name, ByteArray const& v) {
  auto const& value = getValue(name);
  if (!value->data.is<ByteArray>())
    throw NetException("NetSender type mismatch in setData");

  if (value->data.cget<ByteArray>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<ByteArray>(v);
  }
}

void NetVersionSender::setVariant(StringRef name, Variant const& v) {
  auto const& value = getValue(name);
  if (!value->data.is<Variant>())
    throw NetException("NetSender type mismatch in setVariant");

  if (value->data.cget<Variant>() != v) {
    value->lastUpdated = m_currentVersion;
    value->data.set<Variant>(v);
  }
}

size_t NetVersionSender::writeFull(DataStream& ds) const {
  size_t bytesWritten = 0;

  for (auto id : m_schema.ids()) {
    auto const& value = m_values.value(id);
    auto const& field = m_schema.fieldInfo(id);

    bytesWritten += writeValue(ds, field, value);
  }
  return bytesWritten;
}

ByteArray NetVersionSender::writeFull() const {
  DataStreamBuffer buffer;
  writeFull(buffer);
  return buffer.data();
}

size_t NetVersionSender::writeDelta(DataStream& ds, uint64_t sinceVersion) const {
  size_t bytesWritten = 0;

  for (auto const& pair : m_values) {
    auto const& value = pair.second;
    if (value->lastUpdated < sinceVersion)
      continue;

    auto const& field = m_schema.fieldInfo(pair.first);

    bytesWritten += ds.writeVlqU(pair.first);
    bytesWritten += writeValue(ds, field, value);
  }

  return bytesWritten;
}

ByteArray NetVersionSender::writeDelta(uint64_t sinceVersion) const {
  DataStreamBuffer buffer;
  writeDelta(buffer, sinceVersion);
  return buffer.data();
}

NetVersionSender::ValuePtr const& NetVersionSender::getValue(StringRef name) const {
  auto i = m_valuesByName.find(name);
  if (i == m_valuesByName.end())
    throw NetException(strf("No such NetSender state named '%s'", name));
  return i->second;
}

size_t NetVersionSender::writeValue(DataStream& ds, NetFieldInfo const& fieldInfo, ValuePtr const& value) const {
  StreamOffset pos = ds.pos();

  DataPoint data;
  if (fieldInfo.fixedPointBase != 0.0) {
    if (netTypeIsSignedInt(fieldInfo.type))
      data.set<int64_t>(value->data.get<double>() / fieldInfo.fixedPointBase);
    else
      data.set<uint64_t>(value->data.get<double>() / fieldInfo.fixedPointBase);
  } else {
    data = value->data;
  }

  if (fieldInfo.type == NetType::Event) {
    ds.writeVlqU(data.get<uint64_t>());
  } else if (fieldInfo.type == NetType::Int8) {
    ds.write<int8_t>(data.get<int64_t>());
  } else if (fieldInfo.type == NetType::UInt8) {
    ds.write<uint8_t>(data.get<uint64_t>());
  } else if (fieldInfo.type == NetType::Int16) {
    ds.write<int16_t>(data.get<int64_t>());
  } else if (fieldInfo.type == NetType::UInt16) {
    ds.write<uint16_t>(data.get<uint64_t>());
  } else if (fieldInfo.type == NetType::VarInt) {
    ds.writeVlqI(data.get<int64_t>());
  } else if (fieldInfo.type == NetType::VarUInt) {
    ds.writeVlqU(data.get<uint64_t>());
  } else if (fieldInfo.type == NetType::Size) {
    size_t v = data.get<size_t>();
    ds.writeVlqU(v == NPos ? 0 : v + 1);
  } else if (fieldInfo.type == NetType::Float) {
    ds.write<float>(data.get<double>());
  } else if (fieldInfo.type == NetType::Double) {
    ds.write<double>(data.get<double>());
  } else if (fieldInfo.type == NetType::NFloat8) {
    ds.write<uint8_t>(std::round(data.get<double>() * 255.0f));
  } else if (fieldInfo.type == NetType::NFloat16) {
    ds.write<uint16_t>(std::round(data.get<double>() * 65535.0f));
  } else if (fieldInfo.type == NetType::Bool) {
    ds.write<bool>(data.get<bool>());
  } else if (fieldInfo.type == NetType::String) {
    ds.write(data.get<String>());
  } else if (fieldInfo.type == NetType::Data) {
    ds.write(data.get<ByteArray>());
  } else if (fieldInfo.type == NetType::Variant) {
    ds.write(data.get<Variant>());
  }

  return ds.pos() - pos;
}

NetSyncSender::NetSyncSender() {}

NetSyncSender::NetSyncSender(NetSchema const& schema) {
  initialize(schema);
}

size_t NetSyncSender::writeDelta(DataStream& ds) {
  size_t size = NetVersionSender::writeDelta(ds, currentVersion());
  // Increment the version on write so we only write newly written states.
  incrementVersion(currentVersion() + 1);
  return size;
}

size_t NetSyncSender::writeFull(DataStream& ds) {
  size_t size = NetVersionSender::writeFull(ds);
  // Increment the version on write so we only write newly written states.
  incrementVersion(currentVersion() + 1);
  return size;
}

ByteArray NetSyncSender::writeFull() {
  DataStreamBuffer buffer;
  writeFull(buffer);
  return buffer.data();
}

ByteArray NetSyncSender::writeDelta() {
  DataStreamBuffer buffer;
  writeDelta(buffer);
  return buffer.data();
}

NetReceiver::NetReceiver()
  : m_interpolationEnabled(false), m_interpolationStep(0.0) {}

NetReceiver::NetReceiver(NetSchema const& schema)
  : NetReceiver() {
  initialize(schema);
}

void NetReceiver::initialize(NetSchema const& schema) {
  if (!schema.isFinalized())
    throw NetException("schema unfinalized in NetReceiver::initialize");

  m_valuesByName.clear();
  m_values.clear();
  m_schema = schema;
  for (auto id : m_schema.ids()) {
    ValuePtr value = std::make_shared<Value>();
    value->needsForwarding = false;
    value->exactUpdated = false;

    // Initialize with default values because NetSender assumes these as the
    // starting values without sending any data
    auto const& fieldInfo = m_schema.fieldInfo(id);
    if (fieldInfo.fixedPointBase != 0.0) {
      value->latestData =  0.0;
    } else if (fieldInfo.type == NetType::Event) {
      value->latestData = (uint64_t)0;
    } else if (netTypeIsSignedInt(fieldInfo.type)) {
      value->latestData = (int64_t)0;
    } else if (netTypeIsUnsignedInt(fieldInfo.type)) {
      value->latestData = (uint64_t)0;
    } else if (fieldInfo.type == NetType::Size) {
      value->latestData = (size_t)NPos;
    } else if (netTypeIsFloat(fieldInfo.type)) {
      value->latestData = 0.0;
    } else if (fieldInfo.type == NetType::Bool) {
      value->latestData = false;
    } else if (fieldInfo.type == NetType::String) {
      value->latestData = String();
    } else if (fieldInfo.type == NetType::Data) {
      value->latestData = ByteArray();
    } else if (fieldInfo.type == NetType::Variant) {
      value->latestData = Variant();
    }

    value->exactData = value->latestData;
    value->exactUpdated = false;
    if (m_interpolationEnabled) {
      value->stream.reset();
      value->stream.appendDataPoint(m_interpolationStep, value->latestData);
    }

    m_values.insert(id, value);
    m_valuesByName.insert(m_schema.name(id), value);
  }
}

NetSchema const& NetReceiver::schema() const {
  return m_schema;
}

void NetReceiver::enableInterpolation(double startStep, double extrapolation) {
  m_interpolationEnabled = true;
  m_interpolationStep = startStep;
  for (auto const& pair : m_values) {
    pair.second->stream.reset();
    pair.second->stream.setExtrapolation(extrapolation);
    pair.second->stream.appendDataPoint(startStep, pair.second->latestData);
    pair.second->stream.setStep(startStep);
    if (pair.second->exactData != pair.second->latestData) {
      pair.second->exactData = pair.second->latestData;
      pair.second->exactUpdated = true;
    }
  }
}

void NetReceiver::disableInterpolation() {
  m_interpolationEnabled = false;
  m_interpolationStep = 0.0;
  for (auto const& pair : m_values) {
    pair.second->stream.reset();
    if (pair.second->exactData != pair.second->latestData) {
      pair.second->exactData = pair.second->latestData;
      pair.second->exactUpdated = true;
    }
  }
}

void NetReceiver::setInterpolationDiffFunction(StringRef name, DifferenceFunction diffFunction) {
  getValue(name)->diffFunction = diffFunction;
}

void NetReceiver::setInterpolationStep(double step) {
  if (!m_interpolationEnabled)
    return;

  m_interpolationStep = step;
  for (auto const& pair : m_values) {
    pair.second->stream.setStep(step);
    auto data = pair.second->stream.exact();
    if (data != pair.second->exactData) {
      pair.second->exactData = data;
      pair.second->exactUpdated = true;
    }
  }
}

void NetReceiver::readFull(DataStream& ds, size_t dataSize) {
  int bytesLeft = dataSize;

  for (unsigned id : m_schema.ids()) {
    auto const& field = m_schema.fieldInfo(id);
    bytesLeft -= readValue(ds, field, m_values[id], m_interpolationStep);

    if (bytesLeft < 0)
      throw NetException("Read past end of dataSize in NetReceiver::readFull!");
  }

  if (bytesLeft != 0)
    throw NetException("Extra data at end of NetReceiver::readFull!");
}

void NetReceiver::readFull(ByteArray const& fullState) {
  DataStreamBuffer buffer(fullState);
  readFull(buffer, buffer.size());
}

void NetReceiver::readDelta(DataStream& ds, size_t dataSize, double interpolationStep) {
  int bytesLeft = dataSize;

  while (bytesLeft > 0) {
    uint64_t id;
    bytesLeft -= ds.readVlqU(id);
    bytesLeft -= readValue(ds, m_schema.fieldInfo(id), m_values[id], interpolationStep);

    // TODO: This is pretty terrible, we shouldn't read extra *then* fail
    if (bytesLeft < 0)
      throw NetException("Read past end of dataSize in NetReceiver::readDelta!");
  }
}

void NetReceiver::readDelta(ByteArray const& deltaState, double interpolationStep) {
  DataStreamBuffer buffer(deltaState);
  readDelta(buffer, buffer.size(), interpolationStep);
}

void NetReceiver::interpolationHeartbeat(double interpolationStep) {
  if (m_interpolationEnabled) {
    for (auto const& pair : m_values)
      pair.second->stream.heartbeat(interpolationStep);
  }
}

bool NetReceiver::updated(StringRef name) const {
  return getValue(name)->exactUpdated;
}

bool NetReceiver::eventOccurred(StringRef eventName) {
  auto const& value = getValue(eventName);
  if (value->exactUpdated) {
    value->exactUpdated = false;
    return true;
  } else {
    return false;
  }
}

int64_t NetReceiver::getInt(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<int64_t>();
}

uint64_t NetReceiver::getUInt(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<uint64_t>();
}

size_t NetReceiver::getSize(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<size_t>();
}

double NetReceiver::getFloat(StringRef name, bool interpolateIfAvailable) {
  auto const& value = getValue(name);
  value->exactUpdated = false;

  if (m_interpolationEnabled && interpolateIfAvailable) {
    auto weighting = value->stream.weighting();
    auto pointMin = weighting.pointMin.cget<double>();
    auto pointMax = weighting.pointMax.cget<double>();

    if (value->diffFunction)
      return pointMin + value->diffFunction(pointMax, pointMin) * weighting.offset;
    else
      return pointMin + (pointMax - pointMin) * weighting.offset;
  } else {
    return value->exactData.cget<double>();
  }
}

bool NetReceiver::getBool(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<bool>();
}

String NetReceiver::getString(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<String>();
}

ByteArray NetReceiver::getData(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<ByteArray>();
}

Variant NetReceiver::getVariant(StringRef name) {
  auto const& value = getValue(name);
  value->exactUpdated = false;
  return value->exactData.cget<Variant>();
}

void NetReceiver::forward(NetVersionSender& sender, bool forceAll) {
  for (auto id : m_schema.ids()) {
    auto const& value = m_values.get(id);
    auto const& fieldInfo = m_schema.fieldInfo(id);
    if (!forceAll && !value->needsForwarding)
      continue;

    if (fieldInfo.fixedPointBase != 0.0) {
      sender.setFloat(fieldInfo.name, value->latestData.cget<double>());
    } else if (fieldInfo.type == NetType::Event) {
      if (value->needsForwarding)
        sender.triggerEvent(fieldInfo.name);
    } else if (netTypeIsSignedInt(fieldInfo.type)) {
      sender.setInt(fieldInfo.name, value->latestData.cget<int64_t>());
    } else if (netTypeIsUnsignedInt(fieldInfo.type)) {
      sender.setUInt(fieldInfo.name, value->latestData.cget<uint64_t>());
    } else if (fieldInfo.type == NetType::Size) {
      sender.setSize(fieldInfo.name, value->latestData.cget<size_t>());
    } else if (netTypeIsFloat(fieldInfo.type)) {
      sender.setFloat(fieldInfo.name, value->latestData.cget<double>());
    } else if (fieldInfo.type == NetType::Bool) {
      sender.setBool(fieldInfo.name, value->latestData.cget<bool>());
    } else if (fieldInfo.type == NetType::String) {
      sender.setString(fieldInfo.name, value->latestData.cget<String>());
    } else if (fieldInfo.type == NetType::Data) {
      sender.setData(fieldInfo.name, value->latestData.cget<ByteArray>());
    } else if (fieldInfo.type == NetType::Variant) {
      sender.setVariant(fieldInfo.name, value->latestData.cget<Variant>());
    }

    value->needsForwarding = false;
  }
}

NetReceiver::ValuePtr const& NetReceiver::getValue(StringRef name) const {
  auto i = m_valuesByName.find(name);
  if (i == m_valuesByName.end())
    throw NetException(strf("No such NetReceiver state named '%s'", name));

  return i->second;
}

size_t NetReceiver::readValue(DataStream& ds, NetFieldInfo const& fieldInfo, ValuePtr const& value, double step) {
  StreamOffset pos = ds.pos();

  DataPoint dataPoint;
  if (fieldInfo.type == NetType::Event) {
    dataPoint.set<uint64_t>(ds.readVlqU());
  } else if (fieldInfo.type == NetType::Int8) {
    dataPoint.set<int64_t>(ds.read<int8_t>());
  } else if (fieldInfo.type == NetType::UInt8) {
    dataPoint.set<uint64_t>(ds.read<uint8_t>());
  } else if (fieldInfo.type == NetType::Int16) {
    dataPoint.set<int64_t>(ds.read<int16_t>());
  } else if (fieldInfo.type == NetType::UInt16) {
    dataPoint.set<uint64_t>(ds.read<uint16_t>());
  } else if (fieldInfo.type == NetType::VarInt) {
    dataPoint.set<int64_t>(ds.readVlqI());
  } else if (fieldInfo.type == NetType::VarUInt) {
    dataPoint.set<uint64_t>(ds.readVlqU());
  } else if (fieldInfo.type == NetType::Size) {
    auto size = ds.readVlqU();
    dataPoint.set<size_t>(size == 0 ? NPos : size - 1);
  } else if (fieldInfo.type == NetType::Float) {
    dataPoint.set<double>(ds.read<float>());
  } else if (fieldInfo.type == NetType::NFloat8) {
    dataPoint.set<double>(ds.read<uint8_t>() / 255.0f);
  } else if (fieldInfo.type == NetType::NFloat16) {
    dataPoint.set<double>(ds.read<uint16_t>() / 65535.0f);
  } else if (fieldInfo.type == NetType::Double) {
    dataPoint.set<double>(ds.read<double>());
  } else if (fieldInfo.type == NetType::Bool) {
    dataPoint.set<bool>(ds.read<bool>());
  } else if (fieldInfo.type == NetType::String) {
    dataPoint.set<String>(ds.read<String>());
  } else if (fieldInfo.type == NetType::Data) {
    dataPoint.set<ByteArray>(ds.read<ByteArray>());
  } else if (fieldInfo.type == NetType::Variant) {
    dataPoint.set<Variant>(ds.read<Variant>());
  }

  if (fieldInfo.fixedPointBase != 0.0) {
    if (netTypeIsSignedInt(fieldInfo.type))
      dataPoint.set<double>(dataPoint.cget<int64_t>() * fieldInfo.fixedPointBase);
    else
      dataPoint.set<double>(dataPoint.cget<uint64_t>() * fieldInfo.fixedPointBase);
  }

  value->latestData = dataPoint;
  value->needsForwarding = true;

  if (m_interpolationEnabled) {
    value->stream.appendDataPoint(step, dataPoint);
    auto data = value->stream.exact();
    if (value->exactData != data) {
      value->exactData = data;
      value->exactUpdated = true;
    }
  } else {
    if (value->exactData != value->latestData) {
      value->exactData = value->latestData;
      value->exactUpdated = true;
    }
  }

  return ds.pos() - pos;
}

}
