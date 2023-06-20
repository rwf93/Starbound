#ifndef STAR_STREAMING_VIDEO_CONTROLLER_HPP
#define STAR_STREAMING_VIDEO_CONTROLLER_HPP

#include "StarVariantExtra.hpp"

namespace Star {

struct StreamingVideoFrameBuffer {
  StreamingVideoFrameBuffer() : width(), height(), buffer() {}
  unsigned width;
  unsigned height;
  void* buffer;
};

class StreamingVideoController;
typedef shared_ptr<StreamingVideoController> StreamingVideoControllerPtr;

class StreamingVideoController {
public:
  StreamingVideoController();
  virtual ~StreamingVideoController();

  virtual void setStreamConfiguration(Variant const& configuration);

  virtual bool active() const;

  virtual void update();

  virtual void start();
  virtual void stop();

  virtual bool hasError();
  virtual String nextError();
  virtual bool hasStatus();
  virtual String nextStatus();

  virtual void setFrameBufferSize(Vec2U const& size);
  virtual void setStreamMetadata(Variant const& metadata);

  virtual bool nextFrameExpected();
  virtual StreamingVideoFrameBuffer acquireFrameBuffer();
  virtual void submitFrameBuffer(StreamingVideoFrameBuffer buffer);
protected:
  virtual String kind() const;
private:
  StreamingVideoControllerPtr m_impl;
  List<String> m_errors;
  Vec2U m_size;
};


}

#endif
