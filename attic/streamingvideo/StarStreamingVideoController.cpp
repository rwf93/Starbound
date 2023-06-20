#include "StarStreamingVideoController.hpp"

#include "StarTwitchStreamingVideoController.hpp"

namespace Star {

StreamingVideoController::StreamingVideoController() {

}

StreamingVideoController::~StreamingVideoController() {

}

void StreamingVideoController::setStreamConfiguration(Variant const& configuration) {
  auto kind = configuration.getString("kind");
  if (!m_impl || m_impl->kind() != kind) {
    if (kind.equals("dummy", String::CaseInsensitive))
      m_impl = {};
    else if (kind.equals("twitch", String::CaseInsensitive))
      m_impl = make_shared<TwitchStreamingVideoController>();
  }
  if (m_impl) {
    m_impl->setFrameBufferSize(m_size);
    m_impl->setStreamConfiguration(configuration);
  }
}

bool StreamingVideoController::active() const {
  if (m_impl)
    return m_impl->active();
  return false;
}

void StreamingVideoController::update() {
  if (m_impl)
    m_impl->update();
}

void StreamingVideoController::start() {
  if (m_impl)
    m_impl->start();
  else
    m_errors.append("Dummy video controller");
}

void StreamingVideoController::stop() {
  if (m_impl)
    m_impl->stop();
}

bool StreamingVideoController::hasError() {
  if (m_impl)
    return m_impl->hasError();
  return m_errors.size() != 0;
}

String StreamingVideoController::nextError() {
  if (m_impl)
    return m_impl->nextError();
  return m_errors.takeFirst();
}

bool StreamingVideoController::hasStatus() {
  if (m_impl)
    return m_impl->hasStatus();
  return false;
}

String StreamingVideoController::nextStatus() {
  if (m_impl)
    return m_impl->nextStatus();
  starAssert(false);
  return "";
}

void StreamingVideoController::setFrameBufferSize(Vec2U const& size) {
  if (m_impl)
    m_impl->setFrameBufferSize(size);
  m_size = size;
}

void StreamingVideoController::setStreamMetadata(Variant const& metadata) {
  if (m_impl)
    m_impl->setStreamMetadata(metadata);
}

bool StreamingVideoController::nextFrameExpected() {
  if (m_impl)
    return m_impl->nextFrameExpected();
  return false;
}

StreamingVideoFrameBuffer StreamingVideoController::acquireFrameBuffer() {
  if (m_impl)
    return m_impl->acquireFrameBuffer();
  starAssert(false);
  return {};
}

void StreamingVideoController::submitFrameBuffer(StreamingVideoFrameBuffer buffer) {
  if (m_impl)
    m_impl->submitFrameBuffer(buffer);
  else
    starAssert(false);
}

String StreamingVideoController::kind() const {
  if (m_impl)
    return m_impl->kind();
  return "dummy";
}


}
