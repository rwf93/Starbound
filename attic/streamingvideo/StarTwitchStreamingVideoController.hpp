#ifndef STAR_TWITCH_STREAMING_VIDEO_CONTROLLER_HPP
#define STAR_TWITCH_STREAMING_VIDEO_CONTROLLER_HPP

#include "StarStreamingVideoController.hpp"

namespace Star {

class TwitchStreamingVideoControllerInternals;

class TwitchStreamingVideoController: public StreamingVideoController {
public:
  TwitchStreamingVideoController();
  virtual ~TwitchStreamingVideoController();

  virtual void setStreamConfiguration(Variant const& configuration) override;

  virtual bool active() const override;

  virtual void update() override;

  virtual void start() override;
  virtual void stop() override;

  virtual bool hasError() override;
  virtual String nextError() override;
  virtual bool hasStatus() override;
  virtual String nextStatus() override;

  virtual void setFrameBufferSize(Vec2U const& size) override;
  virtual void setStreamMetadata(Variant const& metadata) override;

  virtual bool nextFrameExpected() override;
  virtual StreamingVideoFrameBuffer acquireFrameBuffer() override;
  virtual void submitFrameBuffer(StreamingVideoFrameBuffer buffer) override;
protected:
  virtual String kind() const override;
private:
  float m_timer;
  shared_ptr<TwitchStreamingVideoControllerInternals> m_internals;
};


}

#endif
