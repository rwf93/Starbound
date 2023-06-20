#ifndef STAR_FIFO_HPP
#define STAR_FIFO_HPP

#include "StarIODevice.hpp"
#include "StarThread.hpp"

namespace Star {

STAR_EXCEPTION(FifoClosedException, DeviceClosedException);

STAR_CLASS(Fifo);

// First in first out IODevice.  Bytes written to the device can be read from
// it first byte in, first byte out.  If it set to blocking, then reads that
// read past the end of the available bytes -- or writes that go over the
// available buffer size -- will block until write is called, or the device is
// closed.
//
// A paired set of Fifos work just like a single Fifo, except each Fifo writes
// to the other and reads from the other.
class Fifo : public IODevice {
public:
  // If either one of the paired Fifo objects is destroyed, the other one will
  // immediately be marked as closed.
  static pair<FifoPtr, FifoPtr> makePair(bool blocking = false, size_t bufferSize = 1024);

  Fifo(bool blocking = false, size_t bufferSize = 1024);

  // If this is a paired fifo, then this will close both sides.
  void close() override;

  size_t read(char* data, size_t len) override;
  size_t write(char const* data, size_t len) override;

  String deviceName() const override;

private:
  size_t doWrite(char const* data, size_t len);

  // Disconnects the other side of a paired fifo.
  void disconnect();

  Mutex m_mutex;
  ConditionVariable m_condition;

  ByteArray m_buffer;
  bool m_blocking;
  size_t m_readPos;
  size_t m_writePos;

  bool m_writeFull;
  bool m_readEmpty;

  bool m_isPaired;
  weak_ptr<Fifo> m_pairedFifo;
};

}

#endif
