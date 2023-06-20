#include "StarCellularLiquid.hpp"
#include "StarRandom.hpp"
#include "StarRenderer.hpp"
#include "StarRootLoader.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarAssets.hpp"
#include "StarTextPainter.hpp"
#include "StarMainApplication.hpp"

using namespace Star;

constexpr int LiquidArrayWidth = 60;
constexpr int LiquidArrayHeight = 60;
constexpr int LiquidArraySlowBorder = 0;
constexpr int LiquidArraySlowTickLimit = 200;
constexpr float LiquidBottomDrainPercentage = 0.5f;

class LiquidTest : public Application {
protected:
  struct LiquidStore {
    bool collision;
    bool endless;
    Maybe<size_t> liquid;
    float level;
    float pressure;
  };

  void startup(StringList const& cmdLineArgs) override {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Info, false, {}});
    m_root = rootLoader.initOrDie(cmdLineArgs).first;

    m_liquidArray.resize(Array2S(LiquidArrayWidth, LiquidArrayHeight), LiquidStore());
    m_paused = false;
    m_step = false;
    m_update = 0;

    struct CellWorld : CellularLiquidWorld<size_t> {
      LiquidTest& parent;

      CellWorld(LiquidTest& parent) : parent(parent) {}

      float drainLevel(Vec2I const& location) const override {
        return location[1] == 0 ? LiquidBottomDrainPercentage : 0.0f;
      }

      CellularLiquidCell<size_t> cell(Vec2I const& location) const override {
        if (location[0] < 0 || location[1] < 0 || location[0] >= (int)parent.m_liquidArray.size(0) || location[1] >= (int)parent.m_liquidArray.size(1))
          return {};

        auto const& store = parent.m_liquidArray(location[0], location[1]);
        if (store.collision)
          return CellularLiquidCollisionCell();

        if (store.endless)
          return CellularLiquidSourceCell<size_t>{*store.liquid, store.pressure};
        else
          return CellularLiquidFlowCell<size_t>{store.liquid, store.level, store.pressure};
      }

      void setFlow(Vec2I const& location, CellularLiquidFlowCell<size_t> const& flow) override {
        if (location[0] < 0 || location[1] < 0 || location[0] >= (int)parent.m_liquidArray.size(0) || location[1] >= (int)parent.m_liquidArray.size(1))
          return;

        auto& store = parent.m_liquidArray(location[0], location[1]);
        if (!store.endless && !store.collision) {
          store.liquid = flow.liquid;
          store.level = flow.level;
          store.pressure = flow.pressure;
        }
      }

      void liquidInteraction(Vec2I const& a, size_t aLiquid, Vec2I const& b, size_t bLiquid) override {
        if (a[0] < 0 || a[1] < 0 || a[0] >= (int)parent.m_liquidArray.size(0) || a[1] >= (int)parent.m_liquidArray.size(1))
          return;

        if (b[0] < 0 || b[1] < 0 || b[0] >= (int)parent.m_liquidArray.size(0) || b[1] >= (int)parent.m_liquidArray.size(1))
          return;

        auto& aCell = parent.m_liquidArray(a[0], a[1]);
        auto& bCell = parent.m_liquidArray(b[0], b[1]);

        if (aLiquid == 0 && bLiquid == 2 && !aCell.endless)
          bCell = LiquidStore{true, false, {}, 0.0f, 0.0f};
        if (aLiquid == 2 && bLiquid == 0 && !bCell.endless)
          aCell = LiquidStore{true, false, {}, 0.0f, 0.0f};
        if (aLiquid == 0 && bLiquid == 1 && !aCell.endless)
          aCell.liquid = 1;
        if (aLiquid == 1 && bLiquid == 0 && !bCell.endless)
          bCell.liquid = 1;
      }
    };

    m_liquidEngine = make_shared<LiquidCellEngine<size_t>>(
        Root::singleton().liquidsDatabase()->liquidEngineParameters(), make_shared<CellWorld>(*this));

    auto assets = Root::singleton().assets();

    m_liquids = {{0, 0, 255, 255}, {0, 255, 0, 255}, {255, 0, 0, 255}};
    m_currentLiquid = 0;

    m_liquidEngine->setLiquidTickDelta(0, 2);
    m_liquidEngine->setLiquidTickDelta(1, 3);
    m_liquidEngine->setLiquidTickDelta(2, 5);

    m_liquidEngine->setProcessingLimit(LiquidArraySlowTickLimit);
    m_liquidEngine->setNoProcessingLimitRegions({RectI(0, 0, LiquidArrayWidth, LiquidArrayHeight).trimmed(LiquidArraySlowBorder)});
  }

  void renderInit(RendererPtr renderer) override {
    Application::renderInit(renderer);
    m_textPainter = make_shared<TextPainter>(m_root->assets()->font("/hobo.ttf")->clone(), renderer);
    m_textPainter->setFontSize(16);
  }

  void update() override {
    if (m_mouseButtonHeld) {
      Vec2I cell = screenToCell(m_mousePos);
      auto& mouseCell = m_liquidArray(cell[0], cell[1]);

      if (*m_mouseButtonHeld == MouseButton::Left) {
        mouseCell = LiquidStore{true, false, {}, 0.0f, 0.0f};
      } else if (*m_mouseButtonHeld == MouseButton::Middle) {
        mouseCell = LiquidStore{false, true, m_currentLiquid, 1.0f, 5.0f};
      } else if (*m_mouseButtonHeld == MouseButton::Right) {
        mouseCell = LiquidStore{false, false, {}, 0.0f, 0.0f};
      }

      m_liquidEngine->visitLocation(cell);
    }

    if (!m_paused || m_step) {
      m_liquidEngine->update();
      ++m_update;
    }

    m_step = false;
  }

  void render() override {
    for (int x = 0; x < LiquidArrayWidth; ++x) {
      for (int y = 0; y < LiquidArrayHeight; ++y) {
        auto const& cell = m_liquidArray(x, y);
        RectF screenRect = cellScreenRect({x, y});
        Vec4B color;
        if (cell.collision)
          color = {255, 255, 255, 255};
        if (cell.endless)
          color = {255, 0, 255, 255};
        else if (cell.liquid)
          color = liquidColor(*cell.liquid, cell.level);

        renderer()->render(renderFlatRect(screenRect, color));
      }
    }

    Vec2I mouseCellPos = screenToCell(m_mousePos);
    auto const& mouseCell = m_liquidArray(mouseCellPos[0], mouseCellPos[1]);

    String hoverText = strf(
        "fps: %s\nactive water cells: %s\nactive acid cells: %s\nactive lava cells: %s\ncell: %s\npaused: %s\ncurrent "
        "liquid: %d\ncell collision: %s\ncell liquid: %d\ncell level: %s\ncell pressure: %s\n",
        appController()->renderFps(), m_liquidEngine->activeCells(0), m_liquidEngine->activeCells(1), m_liquidEngine->activeCells(2),
        mouseCellPos, m_paused, m_currentLiquid, mouseCell.collision, mouseCell.liquid, mouseCell.level, mouseCell.pressure);

    m_textPainter->renderText(hoverText, TextPositioning(Vec2F(0, renderer()->screenSize()[1]), HorizontalAnchor::LeftAnchor, VerticalAnchor::TopAnchor));
  }

  void processInput(InputEvent const& event) override {
    if (auto keyDown = event.ptr<KeyDownEvent>()) {
      if (keyDown->key == Key::P)
        m_paused = !m_paused;
      if (keyDown->key == Key::S)
        m_step = true;
    } else if (auto mouseDown = event.ptr<MouseButtonDownEvent>()) {
      m_mouseButtonHeld = mouseDown->mouseButton;
    } else if (auto mouseWheel = event.ptr<MouseWheelEvent>()) {
      if (mouseWheel->mouseWheel == MouseWheel::Up)
        m_currentLiquid = pmod<int>((int)m_currentLiquid + 1, m_liquids.size());
      else
        m_currentLiquid = pmod<int>((int)m_currentLiquid - 1, m_liquids.size());

    } else if (event.is<MouseButtonUpEvent>()) {
      m_mouseButtonHeld.reset();
    } else if (auto mme = event.ptr<MouseMoveEvent>()) {
      m_mousePos = mme->mousePosition;
    }
  }

private:
  typedef MultiArray<LiquidStore, 2> LiquidArray;

  Vec2F cellScreenDimensions() const {
    Vec2F windowSize(renderer()->screenSize());
    return {windowSize[0] / LiquidArrayWidth, windowSize[1] / LiquidArrayHeight};
  }

  RectF cellScreenRect(Vec2I const& c) const {
    Vec2F cdim = cellScreenDimensions();
    Vec2F ll = Vec2F(cdim[0] * c[0], cdim[1] * c[1]);
    return RectF::withSize(ll, cdim);
  }

  Vec2I screenToCell(Vec2I const& screen) const {
    Vec2F cdim = cellScreenDimensions();
    Vec2I cpos = Vec2I::floor(Vec2F(screen[0] / cdim[0], screen[1] / cdim[1]));
    return Vec2I(clamp(cpos[0], 0, LiquidArrayWidth - 1), clamp(cpos[1], 0, LiquidArrayHeight - 1));
  }

  Vec4B liquidColor(size_t liquid, float level) {
    Vec4B color = m_liquids.at(liquid);
    return Color::v4fToByte(Color::v4bToFloat(color) * level);
  }

  RootUPtr m_root;
  LiquidArray m_liquidArray;
  shared_ptr<LiquidCellEngine<size_t>> m_liquidEngine;
  List<Vec4B> m_liquids;
  bool m_paused;
  bool m_step;
  uint64_t m_update;
  Vec2I m_mousePos;

  RendererPtr m_renderer;
  TextPainterPtr m_textPainter;
  Maybe<MouseButton> m_mouseButtonHeld;
  size_t m_currentLiquid;
};

STAR_MAIN_APPLICATION(LiquidTest);
