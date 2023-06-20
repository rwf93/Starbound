#include "StarFifo.hpp"
#include "StarFormat.hpp"

namespace Star {

pair<FifoPtr, FifoPtr> Fifo::makePair(bool blocking, size_t bufferSize) {
  auto left = make_shared<Fifo>(blocking, bufferSize);
  auto right = make_shared<Fifo>(blocking, bufferSize);

  left->m_isPaired = true;
  left->m_pairedFifo = right;
  right->m_isPaired = true;
  right->m_pairedFifo = left;

  return {left, right};
}

Fifo::Fifo(bool blocking, size_t bufferSize) {
  m_buffer.resize(bufferSize, 0);
  m_blocking = blocking;
  m_readPos = 0;
  m_writePos = 0;
  m_writeFull = false;
  m_readEmpty = true;
  m_isPaired = false;
  setMode(ReadWrite);
}

void Fifo::close() {
  setMode(Closed);
  disconnect();
}

size_t Fifo::read(char* data, size_t len) {
  if (len == 0)
    return 0;

  MutexLocker locker(m_mutex);

  size_t ramt = 0;
  while (isOpen()) {
    while (!m_readEmpty && ramt < len) {
      *data = m_buffer[m_readPos];
      ++data;
      ++ramt;
      m_readPos = (m_readPos + 1) % m_buffer.size();

      m_writeFull = false;
      if (m_readPos == m_writePos)
        m_readEmpty = true;
    }

    // Signal any waiting writes.
    m_condition.signal();

    // If we are blocking, and haven't read all we need, wait for writers.
    if (m_blocking && ramt < len) {
      // Make sure to check that we're not disconnected if we're out of data to
      // read, and need to subsequently close.
      if (m_isPaired && m_pairedFifo.expired()) {
        close();
        throw FifoClosedException("Fifo::read called on closed Fifo");
      } else {
        m_condition.wait(m_mutex);
      }
    } else {
      break;
    }
  }

  if (!isOpen() && ramt == 0)
    throw FifoClosedException("Fifo::read called on closed Fifo");

  return ramt;
}

size_t Fifo::write(char const* data, size_t len) {
  if (len == 0)
    return 0;

  if (m_isPaired) {
    if (auto paired = m_pairedFifo.lock()) {
      return paired->doWrite(data, len);
    } else {
      close();
      throw FifoClosedException("Fifo::write called on closed Fifo");
    }
  } else {
    return doWrite(data, len);
  }
}

String Fifo::deviceName() const {
  return strf("Fifo <%s>", this);
}

size_t Fifo::doWrite(char const* data, size_t len) {
  MutexLocker locker(m_mutex);

  size_t wamt = 0;
  while (isOpen()) {
    while (!m_writeFull && wamt < len) {
      m_buffer[m_writePos] = *data;
      ++data;
      ++wamt;
      m_writePos = (m_writePos + 1) % m_buffer.size();

      m_readEmpty = false;
      if (m_writePos == m_readPos)
        m_writeFull = true;
    }

    // Signal any waiting reads.
    m_condition.signal();

    // If we are blocking, and haven't written all we need, wait for readers.
    if (m_blocking && wamt < len)
      m_condition.wait(m_mutex);
    else
      break;
  }

  if (!isOpen() && wamt == 0)
    throw FifoClosedException("Fifo::write called on closed Fifo");

  return wamt;
}

void Fifo::disconnect() {
  if (auto paired = m_pairedFifo.lock()) {
    MutexLocker locker(m_mutex);
    m_pairedFifo.reset();
    paired->disconnect();
    // Wake up all readers / writers so they can fail.
    m_condition.signal();
  }
}

}
