#include "StarLabelWidget.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"

namespace Star {

LabelWidget::LabelWidget(String text,
    Color const& color,
    HorizontalAnchor const& hAnchor,
    VerticalAnchor const& vAnchor,
    Maybe<unsigned> wrapWidth,
    Maybe<float> lineSpacing)
  : m_color(color),
    m_hAnchor(hAnchor),
    m_vAnchor(vAnchor),
    m_wrapWidth(std::move(wrapWidth)),
    m_lineSpacing(std::move(lineSpacing)) {
  auto assets = Root::singleton().assets();
  m_fontSize = assets->json("/interface.config:font").getInt("baseSize");
  setText(std::move(text));
}

String const& LabelWidget::text() const {
  return m_text;
}

Maybe<unsigned> LabelWidget::getTextCharLimit() const {
  return m_textCharLimit;
}

void LabelWidget::setText(String newText) {
  m_text = std::move(newText);
  updateTextRegion();
}

void LabelWidget::setFontSize(int fontSize) {
  m_fontSize = fontSize;
  updateTextRegion();
}

void LabelWidget::setColor(Color newColor) {
  m_color = std::move(newColor);
}

void LabelWidget::setAnchor(HorizontalAnchor hAnchor, VerticalAnchor vAnchor) {
  m_hAnchor = hAnchor;
  m_vAnchor = vAnchor;
  updateTextRegion();
}

void LabelWidget::setWrapWidth(Maybe<unsigned> wrapWidth) {
  m_wrapWidth = std::move(wrapWidth);
  updateTextRegion();
}

void LabelWidget::setLineSpacing(Maybe<float> lineSpacing) {
  m_lineSpacing = std::move(lineSpacing);
  updateTextRegion();
}

void LabelWidget::setDirectives(String const& directives) {
  m_processingDirectives = directives;
  updateTextRegion();
}

void LabelWidget::setTextCharLimit(Maybe<unsigned> charLimit) {
  m_textCharLimit = charLimit;
  updateTextRegion();
}

RectI LabelWidget::relativeBoundRect() const {
  return RectI(m_textRegion).translated(relativePosition());
}

RectI LabelWidget::getScissorRect() const {
  return noScissor();
}

void LabelWidget::renderImpl() {
  context()->setFontSize(m_fontSize);
  context()->setFontColor(m_color.toRgba());
  context()->setFontProcessingDirectives(m_processingDirectives);

  if (m_lineSpacing)
    context()->setLineSpacing(*m_lineSpacing);
  else
    context()->setDefaultLineSpacing();

  context()->renderInterfaceText(m_text, {Vec2F(screenPosition()), m_hAnchor, m_vAnchor, m_wrapWidth, m_textCharLimit});

  context()->setFontProcessingDirectives("");
  context()->setDefaultLineSpacing();
}

void LabelWidget::updateTextRegion() {
  context()->setFontSize(m_fontSize);
  context()->setFontColor(m_color.toRgba());
  context()->setFontProcessingDirectives(m_processingDirectives);

  if (m_lineSpacing)
    context()->setLineSpacing(*m_lineSpacing);
  else
    context()->setDefaultLineSpacing();

  m_textRegion = RectI(context()->determineInterfaceTextSize(m_text, {Vec2F(), m_hAnchor, m_vAnchor, m_wrapWidth, m_textCharLimit}));
  setSize(m_textRegion.size());

  context()->setFontProcessingDirectives("");
  context()->setDefaultLineSpacing();
}

}
