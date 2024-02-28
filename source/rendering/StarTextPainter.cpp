#include "StarTextPainter.hpp"
#include "StarJsonExtra.hpp"

#include <regex>

namespace Star {

namespace Text {
  String stripEscapeCodes(String const& s) {
    String regex = strf("\\%s[^;]*%s", CmdEsc, EndEsc);
    return std::regex_replace(s.utf8(), std::regex(regex.utf8()), "");
  }

  String preprocessEscapeCodes(String const& s) {
    bool escape = false;
    auto result = s.utf8();

    size_t escapeStartIdx = 0;
    for (size_t i = 0; i < result.size(); i++) {
      auto& c = result[i];
      if (c == CmdEsc || c == StartEsc) {
        escape = true;
        escapeStartIdx = i;
      }
      if ((c <= SpecialCharLimit) && !(c == StartEsc))
        escape = false;
      if ((c == EndEsc) && escape)
        result[escapeStartIdx] = StartEsc;
    }
    return {result};
  }

  String extractCodes(String const& s) {
    bool escape = false;
    StringList result;
    String escapeCode;
    for (auto c : preprocessEscapeCodes(s)) {
      if (c == StartEsc)
        escape = true;
      if (c == EndEsc) {
        escape = false;
        for (auto command : escapeCode.split(','))
          result.append(command);
        escapeCode = "";
      }
      if (escape && (c != StartEsc))
        escapeCode.append(c);
    }
    if (!result.size())
      return "";
    return "^" + result.join(",") + ";";
  }
}

TextPositioning::TextPositioning() {
  pos = Vec2F();
  hAnchor = HorizontalAnchor::LeftAnchor;
  vAnchor = VerticalAnchor::BottomAnchor;
}

TextPositioning::TextPositioning(Vec2F pos, HorizontalAnchor hAnchor, VerticalAnchor vAnchor,
    Maybe<unsigned> wrapWidth, Maybe<unsigned> charLimit)
  : pos(pos), hAnchor(hAnchor), vAnchor(vAnchor), wrapWidth(wrapWidth), charLimit(charLimit) {}

TextPositioning::TextPositioning(Json const& v) {
  pos = v.opt("position").apply(jsonToVec2F).value();
  hAnchor = HorizontalAnchorNames.getLeft(v.getString("horizontalAnchor", "left"));
  vAnchor = VerticalAnchorNames.getLeft(v.getString("verticalAnchor", "top"));
  wrapWidth = v.optUInt("wrapWidth");
  charLimit = v.optUInt("charLimit");
}

Json TextPositioning::toJson() const {
  return JsonObject{
    {"position", jsonFromVec2F(pos)},
    {"horizontalAnchor", HorizontalAnchorNames.getRight(hAnchor)},
    {"verticalAnchor", VerticalAnchorNames.getRight(vAnchor)},
    {"wrapWidth", jsonFromMaybe(wrapWidth)}
  };
}

TextPositioning TextPositioning::translated(Vec2F translation) const {
  return {pos + translation, hAnchor, vAnchor, wrapWidth, charLimit};
}

TextPainter::TextPainter(FontPtr font, RendererPtr renderer, TextureGroupPtr textureGroup)
  : m_renderer(renderer),
    m_fontTextureGroup(std::move(font), textureGroup),
    m_fontSize(8),
    m_lineSpacing(1.30f),
    m_renderSettings({FontMode::Normal, Vec4B::filled(255)}),
    m_splitIgnore(" \t"),
    m_splitForce("\n\v"),
    m_nonRenderedCharacters("\n\v\r") {}

RectF TextPainter::renderText(String const& s, TextPositioning const& position) {
  if (position.charLimit) {
    unsigned charLimit = *position.charLimit;
    return doRenderText(s, position, true, &charLimit);
  } else {
    return doRenderText(s, position, true, nullptr);
  }
}

RectF TextPainter::renderLine(String const& s, TextPositioning const& position) {
  if (position.charLimit) {
    unsigned charLimit = *position.charLimit;
    return doRenderLine(s, position, true, &charLimit);
  } else {
    return doRenderLine(s, position, true, nullptr);
  }
}

RectF TextPainter::renderGlyph(String::Char c, TextPositioning const& position) {
  return doRenderGlyph(c, position, true);
}

RectF TextPainter::determineTextSize(String const& s, TextPositioning const& position) {
  return doRenderText(s, position, false, nullptr);
}

RectF TextPainter::determineLineSize(String const& s, TextPositioning const& position) {
  return doRenderLine(s, position, false, nullptr);
}

RectF TextPainter::determineGlyphSize(String::Char c, TextPositioning const& position) {
  return doRenderGlyph(c, position, false);
}

int TextPainter::glyphWidth(String::Char c) {
  return m_fontTextureGroup.glyphWidth(c, m_fontSize);
}

int TextPainter::stringWidth(String const& s) {
  int width = 0;
  bool escape = false;

  for (String::Char c : Text::preprocessEscapeCodes(s)) {
    if (c == Text::StartEsc)
      escape = true;
    if (!escape)
      width += glyphWidth(c);
    if (c == Text::EndEsc)
      escape = false;
  }

  return width;
}

StringList TextPainter::wrapText(String const& s, Maybe<unsigned> wrapWidth) {
  String text = Text::preprocessEscapeCodes(s);

  unsigned lineStart = 0; // Where does this line start ?
  unsigned lineCharSize = 0; // how many characters in this line ?
  unsigned linePixelWidth = 0; // How wide is this line so far

  bool inEscapeSequence = false;

  unsigned splitPos = 0; // Where did we last see a place to split the string ?
  unsigned splitWidth = 0; // How wide was the string there ?

  StringList lines; // list of renderable string lines

  // loop through every character in the string
  for (auto character : text) {
    // this up here to deal with the (common) occurance that the first charcter
    // is an escape initiator
    if (character == Text::StartEsc)
      inEscapeSequence = true;

    if (inEscapeSequence) {
      lineCharSize++; // just jump straight to the next character, we don't care what it is.
      if (character == Text::EndEsc)
        inEscapeSequence = false;
    } else {
      lineCharSize++; // assume at least one character if we get here.

      // is this a linefeed / cr / whatever that forces a line split ?
      if (m_splitForce.find(String(character)) != NPos) {
        // knock one off the end because we don't render the CR
        lines.push_back(text.substr(lineStart, lineCharSize - 1));

        lineStart += lineCharSize; // next line starts after the CR
        lineCharSize = 0; // with no characters in it.
        linePixelWidth = 0; // No width
        splitPos = 0; // and no known splits.
      } else {
        int charWidth = glyphWidth(character);

        // is it a place where we might want to split the line ?
        if (m_splitIgnore.find(String(character)) != NPos) {
          splitPos = lineStart + lineCharSize; // this is the character after the space.
          splitWidth = linePixelWidth + charWidth; // the width of the string at
          // the split point, i.e. after the space.
        }

        // would the line be too long if we render this next character ?
        if (wrapWidth && (linePixelWidth + charWidth) > *wrapWidth) {
          // did we find somewhere to split the line ?
          if (splitPos) {
            lines.push_back(text.substr(lineStart, (splitPos - lineStart) - 1));

            unsigned stringEnd = lineStart + lineCharSize;
            lineCharSize = stringEnd - splitPos; // next line has the characters after the space.

            unsigned stringWidth = (linePixelWidth - splitWidth);
            linePixelWidth = stringWidth + charWidth; // and is as wide as the bit after the space.

            lineStart = splitPos; // next line starts after the space
            splitPos = 0; // split is used up.
          } else {
            // don't draw the last character that puts us over the edge
            lines.push_back(text.substr(lineStart, lineCharSize - 1));

            lineStart += lineCharSize - 1; // skip back by one to include that
            // character on the next line.
            lineCharSize = 1; // next line has that character in
            linePixelWidth = charWidth; // and is as wide as that character
          }
        } else {
          linePixelWidth += charWidth;
        }
      }
    }
  }

  // if we hit the end of the string before hitting the end of the line.
  if (lineCharSize > 0)
    lines.push_back(text.substr(lineStart, lineCharSize));

  return lines;
}

unsigned TextPainter::fontSize() const {
  return m_fontSize;
}

void TextPainter::setFontSize(unsigned size) {
  m_fontSize = size;
}

void TextPainter::setLineSpacing(float lineSpacing) {
  m_lineSpacing = lineSpacing;
}

void TextPainter::setMode(FontMode mode) {
  m_renderSettings.mode = mode;
}

void TextPainter::setSplitIgnore(String const& splitIgnore) {
  m_splitIgnore = splitIgnore;
}

void TextPainter::setFontColor(Vec4B color) {
  m_renderSettings.color = std::move(color);
}

void TextPainter::setProcessingDirectives(String directives) {
  m_processingDirectives = std::move(directives);
}

void TextPainter::cleanup(int64_t timeout) {
  m_fontTextureGroup.cleanup(timeout);
}

RectF TextPainter::doRenderText(String const& s, TextPositioning const& position, bool reallyRender, unsigned* charLimit) {
  Vec2F pos = position.pos;
  StringList lines = wrapText(s, position.wrapWidth);

  int height = (lines.size() - 1) * m_lineSpacing * m_fontSize + m_fontSize;

  auto savedRenderSettings = m_renderSettings;
  m_savedRenderSettings = m_renderSettings;

  if (position.vAnchor == VerticalAnchor::BottomAnchor)
    pos[1] += (height - m_fontSize);
  else if (position.vAnchor == VerticalAnchor::VMidAnchor)
    pos[1] += (height - m_fontSize) / 2;

  RectF bounds = RectF::withSize(pos, Vec2F());
  for (auto i : lines) {
    bounds.combine(doRenderLine(i, { pos, position.hAnchor, position.vAnchor }, reallyRender, charLimit));
    pos[1] -= m_fontSize * m_lineSpacing;

    if (charLimit && *charLimit == 0)
      break;
  }

  m_renderSettings = savedRenderSettings;

  return bounds;
}

RectF TextPainter::doRenderLine(String const& s, TextPositioning const& position, bool reallyRender, unsigned* charLimit) {
  String text = s;
  TextPositioning pos = position;

  if (pos.hAnchor == HorizontalAnchor::RightAnchor) {
    auto trimmedString = s;
    if (charLimit)
      trimmedString = s.slice(0, *charLimit);
    pos.pos[0] -= stringWidth(trimmedString);
    pos.hAnchor = HorizontalAnchor::LeftAnchor;
  } else if (pos.hAnchor == HorizontalAnchor::HMidAnchor) {
    auto trimmedString = s;
    if (charLimit)
      trimmedString = s.slice(0, *charLimit);
    unsigned width = stringWidth(trimmedString);
    pos.pos[0] -= std::floor(width / 2);
    pos.hAnchor = HorizontalAnchor::LeftAnchor;
  }

  bool escape = false;
  String escapeCode;
  RectF bounds = RectF::withSize(pos.pos, Vec2F());
  for (String::Char c : text) {
    if (c == Text::StartEsc)
      escape = true;

    if (!escape) {
      if (charLimit) {
        if (*charLimit == 0)
          break;
        else
          --*charLimit;
      }
      RectF glyphBounds = doRenderGlyph(c, pos, reallyRender);
      bounds.combine(glyphBounds);
      pos.pos[0] += glyphBounds.width();
    } else if (c == Text::EndEsc) {
      auto commands = escapeCode.split(',');
      for (auto command : commands) {
        try {
          if (command == "reset") {
            m_renderSettings = m_savedRenderSettings;
          } else if (command == "set") {
            m_savedRenderSettings = m_renderSettings;
          } else if (command == "shadow") {
            m_renderSettings.mode = (FontMode)((int)m_renderSettings.mode | (int)FontMode::Shadow);
          } else if (command == "noshadow") {
            m_renderSettings.mode = (FontMode)((int)m_renderSettings.mode & (-1 ^ (int)FontMode::Shadow));
          } else {
            // expects both #... sequences and plain old color names.
            Color c = jsonToColor(command);
            c.setAlphaF(c.alphaF() * ((float)m_savedRenderSettings.color[3]) / 255);
            m_renderSettings.color = c.toRgba();
          }
        } catch (JsonException&) {
        } catch (ColorException&) {
        }
      }
      escape = false;
      escapeCode = "";
    }
    if (escape && (c != Text::StartEsc))
      escapeCode.append(c);
  }

  return bounds;
}

RectF TextPainter::doRenderGlyph(String::Char c, TextPositioning const& position, bool reallyRender) {
  if (m_nonRenderedCharacters.find(String(c)) != NPos)
    return RectF();
  int width = glyphWidth(c);
  // Offset left by width if right anchored.
  float hOffset = 0;
  if (position.hAnchor == HorizontalAnchor::RightAnchor)
    hOffset = -width;
  else if (position.hAnchor == HorizontalAnchor::HMidAnchor)
    hOffset = -std::floor(width / 2);

  float vOffset = 0;
  if (position.vAnchor == VerticalAnchor::VMidAnchor)
    vOffset = -std::floor((float)m_fontSize / 2);
  else if (position.vAnchor == VerticalAnchor::TopAnchor)
    vOffset = -(float)m_fontSize;

  if (reallyRender) {
    if ((int)m_renderSettings.mode & (int)FontMode::Shadow) {
      Color shadow = Color::Black;
      shadow.setAlpha(m_renderSettings.color[3]);
      renderGlyph(c, position.pos + Vec2F(hOffset, vOffset - 2), m_fontSize, 1, shadow.toRgba(), m_processingDirectives);
      renderGlyph(c, position.pos + Vec2F(hOffset, vOffset - 1), m_fontSize, 1, shadow.toRgba(), m_processingDirectives);
    }

    renderGlyph(c, position.pos + Vec2F(hOffset, vOffset), m_fontSize, 1, m_renderSettings.color, m_processingDirectives);
  }

  return RectF::withSize(position.pos + Vec2F(hOffset, vOffset), {(float)width, (int)m_fontSize});
}

void TextPainter::renderGlyph(String::Char c, Vec2F const& screenPos, unsigned fontSize,
    float scale, Vec4B const& color, String const& processingDirectives) {
  if (!fontSize)
    return;

  auto texture = m_fontTextureGroup.glyphTexture(c, fontSize, processingDirectives);
  m_renderer->render(renderTexturedRect(std::move(texture), Vec2F(screenPos), scale, color, 0.0f));
}

}
