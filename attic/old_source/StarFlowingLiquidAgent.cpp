#include "StarFlowingLiquidAgent.hpp"
#include "StarRoot.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLivingWorldAgent.hpp"
#include "StarLogging.hpp"

namespace Star {

FlowingLiquidAgent::FlowingLiquidAgent() {
  m_livingWorld = {};
}

void FlowingLiquidAgent::bind(LivingWorldFacadePtr world, LivingWorldAgent* livingWorld) {
  this->m_world = world;
  m_livingWorld = livingWorld;
}

void FlowingLiquidAgent::processLocation(Vec2I const& c) {
  auto level = getLiquidLevel(c.x(), c.y());
  int fountainPressure = 0;

  if (level.liquid != Liquid::SolidTileLiquidPseudoId) {
    if (!hasBackground(c.x(), c.y()) && isOcean(c.x(), c.y())) {
      auto liquid = oceanLiquid(c.x(), c.y());
      auto pressure = oceanLiquidPressure(c.x(), c.y());
      if ((level.liquid !=  liquid)||(level.level != pressure)) {
        setLiquidLevel(c.x(), c.y(), LiquidLevel{liquid, pressure});
        level = getLiquidLevel(c.x(), c.y());
      }
    }
  }

  if (level.level >= Liquid::OverflowFullLiquidLevel)
    if (moveLiquidUpwards(c, level)) {
      auto l = getLiquidLevel(c.x(), c.y());
      fountainPressure = (int) level.level - (int) l.level;
      level = l;
    }
  if (level.level > 0)
    if (moveLiquidDown(c, level, true)) {
      level = getLiquidLevel(c.x(), c.y());
      fountainPressure = 0;
    }
  if (level.level >= Liquid::TrivialLevelThreshold) {
    moveLiquidSideWays(c, fountainPressure);
  }
}

bool FlowingLiquidAgent::moveLiquidDown(Vec2I c, LiquidLevel above, bool attemptMoveOut) {
  auto liquidsDatabase = Root::singleton().liquidsDatabase();

  if (above.liquid == Liquid::SolidTileLiquidPseudoId)
    return false;
  auto below = getLiquidLevel(c.x(), c.y() - 1);
  if (below.liquid == Liquid::SolidTileLiquidPseudoId) {
    if (attemptMoveOut)
      moveLiquidOut(c);
    return false;
  }
  if ((c.y() <= 0) && liquidsDatabase->liquidSettings(above.liquid)->endless)
    return false;

  int move = std::max(0,
      std::min(std::min((int) above.level, (Liquid::MaxLiquidLevel - 1) - (int) below.level),
          std::max((Liquid::OverflowFullLiquidLevel - 1) - (int) below.level,
              std::min((Liquid::PressurePerLevel - ((int) below.level - (int) above.level)) / 2,
                  (int) above.level - (Liquid::TrivialLevelThreshold - 1)))));

  move *= std::min(liquidsDatabase->liquidSettings(below.liquid)->downwardsSpeedModifier, liquidsDatabase->liquidSettings(above.liquid)->downwardsSpeedModifier);

  above.level -= move;
  below.level += move;
  below.liquid = above.liquid;

  if (move > 0) {
    moveLiquid(c.x(), c.y(), 0, -1, above, below);
    return true;
  } else {
    if (attemptMoveOut)
      moveLiquidOut(c);
    return false;
  }
}

bool FlowingLiquidAgent::moveLiquidSideWays(Vec2I c, int fountain) {
  fountain /= Liquid::PressurePerLevel;
  auto below = getLiquidLevel(c.x(), c.y() - 1);
  if ((below.liquid != Liquid::SolidTileLiquidPseudoId) && (below.level < Liquid::FullLiquidLevel))
    return false;
  bool result = false;
  int move = 0;
  while (true) {
    move++;
    int bdx = 0;
    int bl = 0;
    int l;
    auto level = getLiquidLevel(c.x(), c.y());
    if (level.liquid == Liquid::SolidTileLiquidPseudoId)
      break;
    int idx = 0;
    int preferredDirection = m_random.randInt(1);

    for (int dx = -1; dx <= 1; dx += 2) {
      l = checkMoveLiquidSideways(c.x() + dx, c.y(), level);
      if ((l > bl) || ((l == bl) && (idx == preferredDirection))) {
        bl = l;
        bdx = dx;
      }
      idx++;
    }
    if (bl == 0)
      break;

    auto source = getLiquidLevel(c.x(), c.y());
    auto target = getLiquidLevel(c.x() + bdx, c.y());

    if ((source.level > bl) && ((int) source.level - bl > (int) target.level + bl))
      bl++;

    if ((fountain > 0) && (m_random.randInt(5 * fountain) != 0))
      bl /= fountain * 10;

    if ((bl >= Liquid::TrivialSidewaysMoveThreshold)
        && (((int) source.level - bl) >= Liquid::TrivialLevelThreshold)
        && (((int) target.level + bl) >= Liquid::TrivialLevelThreshold)) {

      source.level -= bl;
      target.level += bl;
      target.liquid = source.liquid;
      moveLiquid(c.x(), c.y(), bdx, 0, source, target);
      moveLiquidDown(Vec2I(c.x() + bdx, c.y()), getLiquidLevel(c.x() + bdx, c.y()), false);
      result = true;
    }
    else {
       if (target.level <= Liquid::IdleLevelEvaporateThreshold) {
         source.level = std::max(0, source.level - Liquid::IdleLevelEvaporateThreshold);
         setLiquidLevel(c.x(), c.y(), source);
         result = true;
       }
    }
    break;
  }
  return result;
}

int FlowingLiquidAgent::checkMoveLiquidSideways(int x, int y, LiquidLevel level) {
  if (level.liquid == Liquid::SolidTileLiquidPseudoId)
    return 0;
  auto beside = getLiquidLevel(x, y);
  if (beside.liquid == Liquid::SolidTileLiquidPseudoId)
    return 0;
  int result = (((int) level.level - (int) beside.level) * 1.9) / 2;
  auto liquidsDatabase = Root::singleton().liquidsDatabase();
  result *= std::min(liquidsDatabase->liquidSettings(level.liquid)->sidewaysSpeedModifier, liquidsDatabase->liquidSettings(beside.liquid)->sidewaysSpeedModifier);
  if (result > level.level)
    result = level.level;
  if (result <= 0)
    return 0;
  return result;
}

bool FlowingLiquidAgent::moveLiquidUpwards(Vec2I c, LiquidLevel level) {
  if (level.liquid == Liquid::SolidTileLiquidPseudoId)
    return false;
  if (level.level < Liquid::OverflowFullLiquidLevel) {
    return false;
  }
  auto above = getLiquidLevel(c.x(), c.y() + 1);
  if (above.liquid == Liquid::SolidTileLiquidPseudoId) {
    return false;
  }

  int move = std::max(0,
      std::min((int) level.level - (Liquid::OverflowFullLiquidLevel -1),
          (((int) level.level - (int) above.level) - (Liquid::PressurePerLevel)) / 2));

  if (above.liquid == NullLiquidId)
    above.liquid = level.liquid;

  auto liquidsDatabase = Root::singleton().liquidsDatabase();
  move *= std::min(liquidsDatabase->liquidSettings(level.liquid)->upwardsSpeedModifier, liquidsDatabase->liquidSettings(above.liquid)->upwardsSpeedModifier);
  move = std::min(move, std::min((int) level.level, (Liquid::MaxLiquidLevel - 1) - (int) above.level));

  above.level += move;
  level.level -= move;
  above.liquid = level.liquid;

  if ((move >= Liquid::TrivialUpwardsMoveThreshold)
      && (above.level >= Liquid::TrivialLevelThreshold)) {
    moveLiquid(c.x(), c.y(), 0, 1, level, above);
    return true;
  }
  return false;
}

bool FlowingLiquidAgent::moveLiquidOut(Vec2I c) {
  if (!hasBackground(c.x(), c.y()) && !isOcean(c.x(), c.y())) {
    auto current = getLiquidLevel(c.x(), c.y());
  auto liquidsDatabase = Root::singleton().liquidsDatabase();
    auto setting = liquidsDatabase->liquidSettings(current.liquid);
    if (setting->endless)
      return false;
    // Move Liquid into the background (disappear) at the BackgroundFlowPercentage
    current.level *= 1.0f - setting->backgroundFlowPercentage;
    setLiquidLevel(c.x(), c.y(), current);
    return true;
  } else {
    return false;
  }
}

bool FlowingLiquidAgent::sanityCheckLiquid(LiquidLevel const& level) const {
  if (((level.liquid != Liquid::SolidTileLiquidPseudoId)&&(level.liquid != NullLiquidId)) == (level.level == 0)) {
    Logger::warn("Inconsistent liquid data. Liquid:%s Level:%s", level.liquid, level.level);
    return false;
  }
  return true;
}

LiquidLevel FlowingLiquidAgent::getLiquidLevel(int x, int y) {
  LiquidLevel level = m_world->readTileLiquid(x, y);
#ifndef NDEBUG
  sanityCheckLiquid(level);
#endif
  return level;
}

bool FlowingLiquidAgent::hasBackground(int x, int y) {
  return m_world->hasBackground(x, y);
}

bool FlowingLiquidAgent::isOcean(int x, int y) {
  return m_world->isOcean(x, y);
}

LiquidId FlowingLiquidAgent::oceanLiquid(int x, int y) {
  return m_world->oceanLiquid(x, y);
}

uint16_t FlowingLiquidAgent::oceanLiquidPressure(int x, int y) {
  return m_world->oceanLiquidPressure(x, y);
}

void FlowingLiquidAgent::moveLiquid(int x, int y, int dx, int dy,
    LiquidLevel proposedSourceLevel, LiquidLevel proposedTargetLevel) {
  setLiquidLevel(x, y, proposedSourceLevel);
  setLiquidLevel(x + dx, y + dy, proposedTargetLevel);
}

void FlowingLiquidAgent::setLiquidLevel(int x, int y, LiquidLevel level) {
  if (level.level <= Liquid::TrivialLevelThreshold)
    level.level = 0;
  if (level.level == 0)
    level.liquid = NullLiquidId;
  auto prevLevel = getLiquidLevel(x, y);
  auto liquidsDatabase = Root::singleton().liquidsDatabase();
  auto prevLevelSetting = liquidsDatabase->liquidSettings(prevLevel.liquid);
  if (prevLevelSetting->endless)
    return; // can never update a endlesswater tile after generation
  if ((prevLevel.liquid == level.liquid) && (prevLevel.level == level.level))
    return;
  starAssert(prevLevel.liquid != Liquid::SolidTileLiquidPseudoId);

  MaterialId blockId = NullMaterialId;
  MaterialHue blockHueShift = MaterialHue();

  float flow = fabs((float) level.level - (float) prevLevel.level);
  auto effect = liquidsDatabase->liquidSettings(level.liquid);
  level.liquid = effect->flowsAs;
  float chance = effect->blockGenerationChance * fmin((float)flow / (float)Liquid::FullLiquidLevel, 1.0f);
  if (Random::randf() < chance) {
    if (!effect->connectedOnly
        ||(getLiquidLevel(x-1, y).liquid == Liquid::SolidTileLiquidPseudoId)
        ||(getLiquidLevel(x+1, y).liquid == Liquid::SolidTileLiquidPseudoId)
        ||(getLiquidLevel(x, y-1).liquid == Liquid::SolidTileLiquidPseudoId)
        ||(getLiquidLevel(x, y+1).liquid == Liquid::SolidTileLiquidPseudoId)) {
      blockId = Random::randFrom(effect->blockOptions);
    }
  }

  if ((level.liquid != NullLiquidId) && (level.liquid != Liquid::SolidTileLiquidPseudoId) && (prevLevel.liquid != NullLiquidId) && (prevLevel.liquid != Liquid::SolidTileLiquidPseudoId)
      && (level.liquid != prevLevel.liquid)) {
    auto& interaction = effect->liquidInteraction[prevLevel.liquid];
    if (Random::randf() < interaction.blockGenerationChance) {
      blockId = Random::randFrom(interaction.blockOptions);
    } else {
      float chance = flow / (flow + prevLevel.level);
      if (Random::randf() >= chance)
        level.liquid = prevLevel.liquid;
    }
  }

  if ((blockId != NullMaterialId) && (blockId != EmptyMaterialId)) {
    m_world->writeTileMaterial(x, y, blockId, blockHueShift, true);
  } else {
    if (!sanityCheckLiquid(level))
      return;
    m_world->writeTileLiquid(x, y, level);
  }
  m_livingWorld->visitRegion(x, y, true);
}

}
