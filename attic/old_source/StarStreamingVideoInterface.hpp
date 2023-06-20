#ifndef STAR_STREAMINGVIDEO_INTERFACE_HPP
#define STAR_STREAMINGVIDEO_INTERFACE_HPP

#include "StarPane.hpp"

namespace Star {

STAR_CLASS(StreamingVideoController);

STAR_CLASS(StreamingVideoOptionsPane);
class StreamingVideoOptionsPane : public Pane {
public:
  StreamingVideoOptionsPane(StreamingVideoControllerPtr streamingVideoController);
  virtual ~StreamingVideoOptionsPane();

  virtual void update() override;

private:
  StreamingVideoControllerPtr m_streamingVideoController;

  void start();
  void stop();
};

}

#endif
