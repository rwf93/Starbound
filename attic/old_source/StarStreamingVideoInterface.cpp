#include "StarStreamingVideoInterface.hpp"
#include "StarGuiReader.hpp"
#include "StarRoot.hpp"
#include "StarStreamingVideoController.hpp"
#include "StarWrapLabelWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarLogging.hpp"
#include "StarAssets.hpp"

namespace Star {

StreamingVideoOptionsPane::StreamingVideoOptionsPane(StreamingVideoControllerPtr streamingVideoController) {
  m_streamingVideoController = streamingVideoController;
  GuiReader reader;
  reader.registerCallback("close", [=](WidgetPtr) -> bool {hide(); return true;});

  reader.registerCallback("btnStart", [=](WidgetPtr) -> bool {start(); return true;});
  reader.registerCallback("btnStop", [=](WidgetPtr) -> bool {stop(); return true;});

  auto assets = Root::singleton().assets();
  reader.construct(assets->variant("/interface/windowconfig/streamingvideo.config:paneLayout"), this);

  fetchChild<WrapLabelWidget>("lblErrors")->setText("");
  fetchChild<WrapLabelWidget>("lblStatus")->setText("");
}

StreamingVideoOptionsPane::~StreamingVideoOptionsPane() {

}

void StreamingVideoOptionsPane::update() {
  if (m_streamingVideoController->hasError())
    fetchChild<WrapLabelWidget>("lblErrors")->setText(m_streamingVideoController->nextError());
  if (m_streamingVideoController->hasStatus())
    fetchChild<WrapLabelWidget>("lblStatus")->setText(m_streamingVideoController->nextStatus());

  if (m_streamingVideoController->active()) {
    fetchChild<ButtonWidget>("btnStart")->disable();
    fetchChild<ButtonWidget>("btnStop")->enable();
  } else {
    fetchChild<ButtonWidget>("btnStart")->enable();
    fetchChild<ButtonWidget>("btnStop")->disable();
  }
}

void StreamingVideoOptionsPane::start() {
  fetchChild<WrapLabelWidget>("lblErrors")->setText("");
  fetchChild<WrapLabelWidget>("lblStatus")->setText("Start requested.");
  try {
    m_streamingVideoController->setStreamConfiguration(Root::singleton().assets()->variant("/streamingvideo.config"));
    m_streamingVideoController->start();
  }
  catch (std::exception const& e) {
    Logger::error("Error starting streamingvideo: %s", e.what());
    fetchChild<WrapLabelWidget>("lblErrors")->setText(strf("Error starting streamingvideo: %s", e.what()));
  }
}

void StreamingVideoOptionsPane::stop() {
  fetchChild<WrapLabelWidget>("lblErrors")->setText("");
  fetchChild<WrapLabelWidget>("lblStatus")->setText("Stop requested.");
  try {
    m_streamingVideoController->stop();
  }
  catch (std::exception const& e) {
    Logger::error("Error stopping streamingvideo: %s", e.what());
    fetchChild<WrapLabelWidget>("lblErrors")->setText(strf("Error stopping streamingvideo: %s", e.what()));
  }
}

}
