#include "StarFontTextureGroup.hpp"
#include "StarTime.hpp"
#include "StarImageProcessing.hpp"

namespace Star {

FontTextureGroup::FontTextureGroup(FontPtr font, TextureGroupPtr textureGroup)
  : m_font(std::move(font)), m_textureGroup(std::move(textureGroup)) {}

void FontTextureGroup::cleanup(int64_t timeout) {
  int64_t currentTime = Time::monotonicMilliseconds();
  eraseWhere(m_glyphs, [&](auto const& p) { return currentTime - p.second.time > timeout; });
}

TexturePtr FontTextureGroup::glyphTexture(String::Char c, unsigned size) {
  return glyphTexture(c, size, "");
}

TexturePtr FontTextureGroup::glyphTexture(String::Char c, unsigned size, String const& processingDirectives) {
  auto res = m_glyphs.insert(GlyphDescriptor{c, size, processingDirectives}, GlyphTexture());

  if (res.second) {
    m_font->setPixelSize(size);
    Image image = m_font->render(c);
    if (!processingDirectives.empty())
      image = processImageOperations(parseImageOperations(processingDirectives), image);

    res.first->second.texture = m_textureGroup->create(image);
  }

  res.first->second.time = Time::monotonicMilliseconds();
  return res.first->second.texture;
}

unsigned FontTextureGroup::glyphWidth(String::Char c, unsigned fontSize) {
  m_font->setPixelSize(fontSize);
  return m_font->width(c);
}

}
