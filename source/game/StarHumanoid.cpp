#include "StarHumanoid.hpp"
#include "StarRoot.hpp"
#include "StarJsonExtra.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarArmors.hpp"
#include "StarParticleDatabase.hpp"
#include "StarAssets.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarDanceDatabase.hpp"

namespace Star {

extern EnumMap<HumanoidEmote> const HumanoidEmoteNames{
  {HumanoidEmote::Idle, "Idle"},
  {HumanoidEmote::Blabbering, "Blabbering"},
  {HumanoidEmote::Shouting, "Shouting"},
  {HumanoidEmote::Happy, "Happy"},
  {HumanoidEmote::Sad, "Sad"},
  {HumanoidEmote::NEUTRAL, "NEUTRAL"},
  {HumanoidEmote::Laugh, "Laugh"},
  {HumanoidEmote::Annoyed, "Annoyed"},
  {HumanoidEmote::Oh, "Oh"},
  {HumanoidEmote::OOOH, "OOOH"},
  {HumanoidEmote::Blink, "Blink"},
  {HumanoidEmote::Wink, "Wink"},
  {HumanoidEmote::Eat, "Eat"},
  {HumanoidEmote::Sleep, "Sleep"}
};

Personality parsePersonality(Json const& config) {
  return Personality{config.getString(0), config.getString(1), jsonToVec2F(config.get(2)), jsonToVec2F(config.get(3))};
}

HumanoidIdentity::HumanoidIdentity(Json config) {
  if (config.isNull())
    config = JsonObject();

  name = config.getString("name", "Humanoid");
  species = config.getString("species", "human");
  gender = GenderNames.getLeft(config.getString("gender", "male"));
  hairGroup = config.getString("hairGroup", "hair");
  hairType = config.getString("hairType", "male1");
  hairDirectives = config.getString("hairDirectives", "");
  bodyDirectives = config.getString("bodyDirectives", "");
  emoteDirectives = config.getString("emoteDirectives", bodyDirectives);
  facialHairGroup = config.getString("facialHairGroup", "");
  facialHairType = config.getString("facialHairType", "");
  facialHairDirectives = config.getString("facialHairDirectives", "");
  facialMaskGroup = config.getString("facialMaskGroup", "");
  facialMaskType = config.getString("facialMaskType", "");
  facialMaskDirectives = config.getString("facialMaskDirectives", "");

  personality.idle = config.getString("personalityIdle", "idle.1");
  personality.armIdle = config.getString("personalityArmIdle", "idle.1");
  personality.headOffset = jsonToVec2F(config.get("personalityHeadOffset", JsonArray{0, 0}));
  personality.armOffset = jsonToVec2F(config.get("personalityArmOffset", JsonArray{0, 0}));

  color = jsonToColor(config.get("color", "white")).toRgba();

  imagePath = config.optString("imagePath");
}

Json HumanoidIdentity::toJson() const {
  auto result = JsonObject{
    {"name", name},
    {"species", species},
    {"gender", GenderNames.getRight(gender)},
    {"hairGroup", hairGroup},
    {"hairType", hairType},
    {"hairDirectives", hairDirectives},
    {"bodyDirectives", bodyDirectives},
    {"emoteDirectives", emoteDirectives},
    {"facialHairGroup", facialHairGroup},
    {"facialHairType", facialHairType},
    {"facialHairDirectives", facialHairDirectives},
    {"facialMaskGroup", facialMaskGroup},
    {"facialMaskType", facialMaskType},
    {"facialMaskDirectives", facialMaskDirectives},
    {"personalityIdle", personality.idle},
    {"personalityArmIdle", personality.armIdle},
    {"personalityHeadOffset", jsonFromVec2F(personality.headOffset)},
    {"personalityArmOffset", jsonFromVec2F(personality.armOffset)},
    {"color", jsonFromColor(Color::rgba(color))}
  };
  if (imagePath)
    result["imagePath"] = *imagePath;
  return result;
}

DataStream& operator>>(DataStream& ds, HumanoidIdentity& identity) {
  ds.read(identity.name);
  ds.read(identity.species);
  ds.read(identity.gender);
  ds.read(identity.hairGroup);
  ds.read(identity.hairType);
  ds.read(identity.hairDirectives);
  ds.read(identity.bodyDirectives);
  ds.read(identity.emoteDirectives);
  ds.read(identity.facialHairGroup);
  ds.read(identity.facialHairType);
  ds.read(identity.facialHairDirectives);
  ds.read(identity.facialMaskGroup);
  ds.read(identity.facialMaskType);
  ds.read(identity.facialMaskDirectives);
  ds.read(identity.personality.idle);
  ds.read(identity.personality.armIdle);
  ds.read(identity.personality.headOffset);
  ds.read(identity.personality.armOffset);
  ds.read(identity.color);
  ds.read(identity.imagePath);

  return ds;
}

DataStream& operator<<(DataStream& ds, HumanoidIdentity const& identity) {
  ds.write(identity.name);
  ds.write(identity.species);
  ds.write(identity.gender);
  ds.write(identity.hairGroup);
  ds.write(identity.hairType);
  ds.write(identity.hairDirectives);
  ds.write(identity.bodyDirectives);
  ds.write(identity.emoteDirectives);
  ds.write(identity.facialHairGroup);
  ds.write(identity.facialHairType);
  ds.write(identity.facialHairDirectives);
  ds.write(identity.facialMaskGroup);
  ds.write(identity.facialMaskType);
  ds.write(identity.facialMaskDirectives);
  ds.write(identity.personality.idle);
  ds.write(identity.personality.armIdle);
  ds.write(identity.personality.headOffset);
  ds.write(identity.personality.armOffset);
  ds.write(identity.color);
  ds.write(identity.imagePath);

  return ds;
}

Humanoid::HumanoidTiming::HumanoidTiming(Json config) {
  if (config.type() != Json::Type::Object) {
    auto assets = Root::singleton().assets();
    config = assets->json("/humanoid.config:humanoidTiming");
  }

  if (config.contains("stateCycle"))
    stateCycle = jsonToArrayF<STATESIZE>(config.get("stateCycle"));
  if (config.contains("stateFrames"))
    stateFrames = jsonToArrayU<STATESIZE>(config.get("stateFrames"));

  if (config.contains("emoteCycle"))
    emoteCycle = jsonToArrayF<EmoteSize>(config.get("emoteCycle"));
  if (config.contains("emoteFrames"))
    emoteFrames = jsonToArrayU<EmoteSize>(config.get("emoteFrames"));
}

bool Humanoid::HumanoidTiming::cyclicState(State state) {
  switch (state) {
    case State::Walk:
    case State::Run:
    case State::Swim:
      return true;
    default:
      return false;
  }
}

bool Humanoid::HumanoidTiming::cyclicEmoteState(HumanoidEmote state) {
  switch (state) {
    case HumanoidEmote::Blabbering:
    case HumanoidEmote::Shouting:
    case HumanoidEmote::Sad:
    case HumanoidEmote::Laugh:
    case HumanoidEmote::Eat:
    case HumanoidEmote::Sleep:
      return true;
    default:
      return false;
  }
}

int Humanoid::HumanoidTiming::stateSeq(float timer, State state) const {
  return genericSeq(timer, stateCycle[state], stateFrames[state], cyclicState(state));
}

int Humanoid::HumanoidTiming::emoteStateSeq(float timer, HumanoidEmote state) const {
  return genericSeq(timer, emoteCycle[(size_t)state], emoteFrames[(size_t)state], cyclicEmoteState(state));
}

int Humanoid::HumanoidTiming::danceSeq(float timer, DancePtr dance) const {
  return genericSeq(timer, dance->cycle, dance->steps.size(), dance->cyclic) - 1;
}

int Humanoid::HumanoidTiming::genericSeq(float timer, float cycle, unsigned frames, bool cyclic) const {
  if (cyclic) {
    timer = fmod(timer, cycle);
  }
  return clamp<int>(timer * frames / cycle, 0, frames - 1) + 1;
}

EnumMap<Humanoid::State> const Humanoid::StateNames{
    {Humanoid::State::Idle, "idle"},
    {Humanoid::State::Walk, "walk"},
    {Humanoid::State::Run, "run"},
    {Humanoid::State::Jump, "jump"},
    {Humanoid::State::Fall, "fall"},
    {Humanoid::State::Swim, "swim"},
    {Humanoid::State::SwimIdle, "swimIdle"},
    {Humanoid::State::Duck, "duck"},
    {Humanoid::State::Sit, "sit"},
    {Humanoid::State::Lay, "lay"},
};

Humanoid::Humanoid(Json const& config) {
  m_timing = HumanoidTiming(config.getObject("humanoidTiming"));

  m_globalOffset = jsonToVec2F(config.get("globalOffset")) / TilePixels;
  m_headRunOffset = jsonToVec2F(config.get("headRunOffset")) / TilePixels;
  m_headSwimOffset = jsonToVec2F(config.get("headSwimOffset")) / TilePixels;
  m_runFallOffset = config.get("runFallOffset").toDouble() / TilePixels;
  m_duckOffset = config.get("duckOffset").toDouble() / TilePixels;
  m_headDuckOffset = jsonToVec2F(config.get("headDuckOffset")) / TilePixels;
  m_sitOffset = config.get("sitOffset").toDouble() / TilePixels;
  m_layOffset = config.get("layOffset").toDouble() / TilePixels;
  m_headSitOffset = jsonToVec2F(config.get("headSitOffset")) / TilePixels;
  m_headLayOffset = jsonToVec2F(config.get("headLayOffset")) / TilePixels;
  m_recoilOffset = jsonToVec2F(config.get("recoilOffset")) / TilePixels;
  m_mouthOffset = jsonToVec2F(config.get("mouthOffset")) / TilePixels;
  m_feetOffset = jsonToVec2F(config.get("feetOffset")) / TilePixels;

  m_bodyFullbright = config.getBool("bodyFullbright", false);

  m_headArmorOffset = jsonToVec2F(config.get("headArmorOffset")) / TilePixels;
  m_chestArmorOffset = jsonToVec2F(config.get("chestArmorOffset")) / TilePixels;
  m_legsArmorOffset = jsonToVec2F(config.get("legsArmorOffset")) / TilePixels;
  m_backArmorOffset = jsonToVec2F(config.get("backArmorOffset")) / TilePixels;

  m_bodyHidden = config.getBool("bodyHidden", false);

  m_armWalkSeq = jsonToIntList(config.get("armWalkSeq"));
  m_armRunSeq = jsonToIntList(config.get("armRunSeq"));

  for (auto const& v : config.get("walkBob").toArray())
    m_walkBob.append(v.toDouble() / TilePixels);
  for (auto const& v : config.get("runBob").toArray())
    m_runBob.append(v.toDouble() / TilePixels);
  for (auto const& v : config.get("swimBob").toArray())
    m_swimBob.append(v.toDouble() / TilePixels);

  m_jumpBob = config.get("jumpBob").toDouble() / TilePixels;
  m_frontArmRotationCenter = jsonToVec2F(config.get("frontArmRotationCenter")) / TilePixels;
  m_backArmRotationCenter = jsonToVec2F(config.get("backArmRotationCenter")) / TilePixels;
  m_frontHandPosition = jsonToVec2F(config.get("frontHandPosition")) / TilePixels;
  m_backArmOffset = jsonToVec2F(config.get("backArmOffset")) / TilePixels;
  m_vaporTrailFrames = config.get("vaporTrailFrames").toInt();
  m_vaporTrailCycle = config.get("vaporTrailCycle").toFloat();

  m_defaultDeathParticles = config.getString("deathParticles");
  m_particleEmitters = config.get("particleEmitters");

  m_defaultMovementParameters = config.get("movementParameters");

  m_twoHanded = false;
  m_primaryHand.holdingItem = false;
  m_altHand.holdingItem = false;

  m_movingBackwards = false;
  m_altHand.angle = 0;
  m_facingDirection = Direction::Left;
  m_rotation = 0;
  m_drawVaporTrail = false;
  m_state = State::Idle;
  m_emoteState = HumanoidEmote::Idle;
  m_dance = {};
  m_emoteAnimationTimer = 0;

  m_primaryHand.angle = 0;
  m_animationTimer = 0.0f;
}

Humanoid::Humanoid(HumanoidIdentity const& identity)
  : Humanoid(Root::singleton().speciesDatabase()->species(identity.species)->humanoidConfig()) {
  setIdentity(identity);
}

void Humanoid::setIdentity(HumanoidIdentity const& identity) {
  m_identity = identity;
  m_headFrameset = getHeadFromIdentity();
  m_bodyFrameset = getBodyFromIdentity();
  m_emoteFrameset = getFacialEmotesFromIdentity();
  m_hairFrameset = getHairFromIdentity();
  m_facialHairFrameset = getFacialHairFromIdentity();
  m_facialMaskFrameset = getFacialMaskFromIdentity();
  m_backArmFrameset = getBackArmFromIdentity();
  m_frontArmFrameset = getFrontArmFromIdentity();
  m_vaporTrailFrameset = getVaporTrailFrameset();
}

HumanoidIdentity const& Humanoid::identity() const {
  return m_identity;
}

void Humanoid::setHeadArmorDirectives(String directives) {
  m_headArmorDirectives = std::move(directives);
}

void Humanoid::setHeadArmorFrameset(String headFrameset) {
  m_headArmorFrameset = std::move(headFrameset);
}

void Humanoid::setChestArmorDirectives(String directives) {
  m_chestArmorDirectives = std::move(directives);
}

void Humanoid::setChestArmorFrameset(String chest) {
  m_chestArmorFrameset = std::move(chest);
}

void Humanoid::setBackSleeveFrameset(String backSleeveFrameset) {
  m_backSleeveFrameset = std::move(backSleeveFrameset);
}

void Humanoid::setFrontSleeveFrameset(String frontSleeveFrameset) {
  m_frontSleeveFrameset = std::move(frontSleeveFrameset);
}

void Humanoid::setLegsArmorDirectives(String directives) {
  m_legsArmorDirectives = std::move(directives);
}

void Humanoid::setLegsArmorFrameset(String legsFrameset) {
  m_legsArmorFrameset = std::move(legsFrameset);
}

void Humanoid::setBackArmorDirectives(String directives) {
  m_backArmorDirectives = std::move(directives);
}

void Humanoid::setBackArmorFrameset(String backFrameset) {
  m_backArmorFrameset = std::move(backFrameset);
}

void Humanoid::setHelmetMaskDirectives(String helmetMaskDirectives) {
  m_helmetMaskDirectives = std::move(helmetMaskDirectives);
}

void Humanoid::setBodyHidden(bool hidden) {
  m_bodyHidden = hidden;
}

void Humanoid::setState(State state) {
  if (m_state != state)
    m_animationTimer = 0.0f;
  m_state = state;
}

void Humanoid::setEmoteState(HumanoidEmote state) {
  if (m_emoteState != state)
    m_emoteAnimationTimer = 0.0f;
  m_emoteState = state;
}

void Humanoid::setDance(Maybe<String> const& dance) {
  if (m_dance != dance)
    m_danceTimer = 0.0f;
  m_dance = dance;
}

void Humanoid::setFacingDirection(Direction facingDirection) {
  m_facingDirection = facingDirection;
}

void Humanoid::setMovingBackwards(bool movingBackwards) {
  m_movingBackwards = movingBackwards;
}

void Humanoid::setRotation(float rotation) {
  m_rotation = rotation;
}

void Humanoid::setVaporTrail(bool enabled) {
  m_drawVaporTrail = enabled;
}

Humanoid::State Humanoid::state() const {
  return m_state;
}

HumanoidEmote Humanoid::emoteState() const {
  return m_emoteState;
}

Maybe<String> Humanoid::dance() const {
  return m_dance;
}

Direction Humanoid::facingDirection() const {
  return m_facingDirection;
}

bool Humanoid::movingBackwards() const {
  return m_movingBackwards;
}

void Humanoid::setPrimaryHandParameters(bool holdingItem, float angle, float itemAngle, bool twoHanded,
    bool recoil, bool outsideOfHand) {
  m_primaryHand.holdingItem = holdingItem;
  m_primaryHand.angle = angle;
  m_primaryHand.itemAngle = itemAngle;
  m_twoHanded = twoHanded;
  m_primaryHand.recoil = recoil;
  m_primaryHand.outsideOfHand = outsideOfHand;
}

void Humanoid::setPrimaryHandFrameOverrides(String backFrameOverride, String frontFrameOverride) {
  m_primaryHand.backFrame = !backFrameOverride.empty() ? std::move(backFrameOverride) : "rotation";
  m_primaryHand.frontFrame = !frontFrameOverride.empty() ? std::move(frontFrameOverride) : "rotation";
}

void Humanoid::setPrimaryHandDrawables(List<Drawable> drawables) {
  m_primaryHand.itemDrawables = std::move(drawables);
}

void Humanoid::setPrimaryHandNonRotatedDrawables(List<Drawable> drawables) {
  m_primaryHand.nonRotatedDrawables = std::move(drawables);
}

void Humanoid::setAltHandParameters(bool holdingItem, float angle, float itemAngle, bool recoil,
    bool outsideOfHand) {
  m_altHand.holdingItem = holdingItem;
  m_altHand.angle = angle;
  m_altHand.itemAngle = itemAngle;
  m_altHand.recoil = recoil;
  m_altHand.outsideOfHand = outsideOfHand;
}

void Humanoid::setAltHandFrameOverrides(String backFrameOverride, String frontFrameOverride) {
  m_altHand.backFrame = !backFrameOverride.empty() ? std::move(backFrameOverride) : "rotation";
  m_altHand.frontFrame = !frontFrameOverride.empty() ? std::move(frontFrameOverride) : "rotation";
}

void Humanoid::setAltHandDrawables(List<Drawable> drawables) {
  m_altHand.itemDrawables = std::move(drawables);
}

void Humanoid::setAltHandNonRotatedDrawables(List<Drawable> drawables) {
  m_altHand.nonRotatedDrawables = std::move(drawables);
}

void Humanoid::animate(float dt) {
  m_animationTimer += dt;
  m_emoteAnimationTimer += dt;
  m_danceTimer += dt;
}

void Humanoid::resetAnimation() {
  m_animationTimer = 0.0f;
  m_emoteAnimationTimer = 0.0f;
  m_danceTimer = 0.0f;
}

List<Drawable> Humanoid::render() {
  List<Drawable> drawables;

  int armStateSeq = getArmStateSequence();
  int bodyStateSeq = getBodyStateSequence();
  int emoteStateSeq = m_timing.emoteStateSeq(m_emoteAnimationTimer, m_emoteState);
  float bobYOffset = getBobYOffset();
  Maybe<DancePtr> dance = getDance();
  Maybe<DanceStep> danceStep = {};
  if (dance.isValid()) {
    if (!(*dance)->states.contains(StateNames.getRight(m_state)))
      dance = {};
    else
      danceStep = (*dance)->steps[m_timing.danceSeq(m_danceTimer, *dance)];
  }

  auto frontHand = (m_facingDirection == Direction::Left || m_twoHanded) ? m_primaryHand : m_altHand;
  auto backHand = (m_facingDirection == Direction::Right || m_twoHanded) ? m_primaryHand : m_altHand;

  Vec2F frontArmFrameOffset = Vec2F(0, bobYOffset);
  if (frontHand.recoil)
    frontArmFrameOffset += m_recoilOffset;
  Vec2F backArmFrameOffset = Vec2F(0, bobYOffset);
  if (backHand.recoil)
    backArmFrameOffset += m_recoilOffset;

  auto addDrawable = [&](Drawable drawable, bool forceFullbright = false) {
    if (m_facingDirection == Direction::Left)
      drawable.scale(Vec2F(-1, 1));
    drawable.fullbright = drawable.fullbright || forceFullbright;
    drawables.append(std::move(drawable));
  };

  auto backArmDrawable = [&](String const& frameSet, String const& directives) -> Drawable {
    String image = strf("%s:%s%s", frameSet, backHand.backFrame, directives);
    Drawable backArm = Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, backArmFrameOffset);
    backArm.rotate(backHand.angle, backArmFrameOffset + m_backArmRotationCenter + m_backArmOffset);
    return backArm;
  };

  if (!m_backArmorFrameset.empty()) {
    auto frameGroup = frameBase(m_state);
    if (m_movingBackwards && (m_state == State::Run))
      frameGroup = "runbackwards";
    String image;
    if (dance.isValid() && danceStep->bodyFrame)
      image = strf("%s:%s%s", m_backArmorFrameset, *danceStep->bodyFrame, getBackDirectives());
    else if (m_state == Idle)
      image = strf("%s:%s%s", m_backArmorFrameset, m_identity.personality.idle, getBackDirectives());
    else
      image = strf("%s:%s.%s%s", m_backArmorFrameset, frameGroup, bodyStateSeq, getBackDirectives());

    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, Vec2F()));
  }

  if (backHand.holdingItem && !dance.isValid()) {
    auto drawItem = [&]() {
      for (auto backHandItem : backHand.itemDrawables) {
        backHandItem.translate(m_frontHandPosition + backArmFrameOffset + m_backArmOffset);
        backHandItem.rotate(backHand.itemAngle, backArmFrameOffset + m_backArmRotationCenter + m_backArmOffset);
        addDrawable(std::move(backHandItem));
      }
    };
    if (!m_twoHanded && backHand.outsideOfHand)
      drawItem();
    if (!m_backArmFrameset.empty() && !m_bodyHidden)
      addDrawable(backArmDrawable(m_backArmFrameset, getBodyDirectives()), m_bodyFullbright);
    if (!m_backSleeveFrameset.empty())
      addDrawable(backArmDrawable(m_backSleeveFrameset, getChestDirectives()));
    if (!m_twoHanded && !backHand.outsideOfHand)
      drawItem();
  } else {
    if (!m_backArmFrameset.empty() && !m_bodyHidden) {
      String image;
      Vec2F position;
      if (dance.isValid() && danceStep->backArmFrame) {
        image = strf("%s:%s%s", m_backArmFrameset, *danceStep->backArmFrame, getBodyDirectives());
        position = danceStep->backArmOffset / TilePixels;
      } else if (m_state == Idle) {
        image = strf("%s:%s%s", m_backArmFrameset, m_identity.personality.armIdle, getBodyDirectives());
        position = m_identity.personality.armOffset / TilePixels;
      } else
        image = strf("%s:%s.%s%s", m_backArmFrameset, frameBase(m_state), armStateSeq, getBodyDirectives());
      auto drawable = Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, position);
      if (dance.isValid())
        drawable.rotate(danceStep->backArmRotation);
      addDrawable(drawable, m_bodyFullbright);
    }
    if (!m_backSleeveFrameset.empty()) {
      String image;
      Vec2F position;
      if (dance.isValid() && danceStep->backArmFrame) {
        image = strf("%s:%s%s", m_backSleeveFrameset, *danceStep->backArmFrame, getChestDirectives());
        position = danceStep->backArmOffset / TilePixels;
      } else if (m_state == Idle) {
        image = strf("%s:%s%s", m_backSleeveFrameset, m_identity.personality.armIdle, getChestDirectives());
        position = m_identity.personality.armOffset / TilePixels;
      } else
        image = strf("%s:%s.%s%s", m_backSleeveFrameset, frameBase(m_state), armStateSeq, getChestDirectives());
      auto drawable = Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, position);
      if (dance.isValid())
        drawable.rotate(danceStep->backArmRotation);
      addDrawable(drawable);
    }
  }

  Vec2F headPosition(0, bobYOffset);
  if (dance.isValid())
    headPosition += danceStep->headOffset / TilePixels;
  else if (m_state == Idle)
    headPosition += m_identity.personality.headOffset / TilePixels;
  else if (m_state == Run)
    headPosition += m_headRunOffset;
  else if (m_state == Swim || m_state == SwimIdle)
    headPosition += m_headSwimOffset;
  else if (m_state == Duck)
    headPosition += m_headDuckOffset;
  else if (m_state == Sit)
    headPosition += m_headSitOffset;
  else if (m_state == Lay)
    headPosition += m_headLayOffset;

  if (!m_headFrameset.empty() && !m_bodyHidden) {
    String image = strf("%s:normal%s", m_headFrameset, getBodyDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition), m_bodyFullbright);
  }

  if (!m_emoteFrameset.empty() && !m_bodyHidden) {
    String image = strf("%s:%s.%s%s", m_emoteFrameset, emoteFrameBase(m_emoteState), emoteStateSeq, getEmoteDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition), m_bodyFullbright);
  }

  if (!m_hairFrameset.empty() && !m_bodyHidden) {
    String image = strf("%s:normal%s", m_hairFrameset, getHairDirectives() + getHelmetMaskDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition), m_bodyFullbright);
  }

  if (!m_bodyFrameset.empty() && !m_bodyHidden) {
    String image;
    if (dance.isValid() && danceStep->bodyFrame)
      image = strf("%s:%s%s", m_bodyFrameset, *danceStep->bodyFrame, getBodyDirectives());
    else if (m_state == Idle)
      image = strf("%s:%s%s", m_bodyFrameset, m_identity.personality.idle, getBodyDirectives());
    else
      image = strf("%s:%s.%s%s", m_bodyFrameset, frameBase(m_state), bodyStateSeq, getBodyDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, {}), m_bodyFullbright);
  }

  if (!m_legsArmorFrameset.empty()) {
    String image;
    if (dance.isValid() && danceStep->bodyFrame)
      image = strf("%s:%s%s", m_legsArmorFrameset, *danceStep->bodyFrame, getLegsDirectives());
    else if (m_state == Idle)
      image = strf("%s:%s%s", m_legsArmorFrameset, m_identity.personality.idle, getLegsDirectives());
    else
      image = strf("%s:%s.%s%s", m_legsArmorFrameset, frameBase(m_state), bodyStateSeq, getLegsDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, {}));
  }

  if (!m_chestArmorFrameset.empty()) {
    String image;
    Vec2F position;
    if (dance.isValid() && danceStep->bodyFrame)
      image = strf("%s:%s%s", m_chestArmorFrameset, *danceStep->bodyFrame, getChestDirectives());
    else if (m_state == Run)
      image = strf("%s:run%s", m_chestArmorFrameset, getChestDirectives());
    else if (m_state == Idle)
      image = strf("%s:%s%s", m_chestArmorFrameset, m_identity.personality.idle, getChestDirectives());
    else if (m_state == Duck)
      image = strf("%s:duck%s", m_chestArmorFrameset, getChestDirectives());
    else if ((m_state == Swim) || (m_state == SwimIdle))
      image = strf("%s:swim%s", m_chestArmorFrameset, getChestDirectives());
    else
      image = strf("%s:chest.1%s", m_chestArmorFrameset, getChestDirectives());
    if (m_state != Duck)
      position[1] += bobYOffset;
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, position));
  }

  if (!m_facialHairFrameset.empty() && !m_bodyHidden) {
    String image = strf("%s:normal%s", m_facialHairFrameset, getFacialHairDirectives() + getHelmetMaskDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition), m_bodyFullbright);
  }

  if (!m_facialMaskFrameset.empty() && !m_bodyHidden) {
    String image = strf("%s:normal%s", m_facialMaskFrameset, getFacialMaskDirectives() + getHelmetMaskDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition));
  }

  if (!m_headArmorFrameset.empty()) {
    String image = strf("%s:normal%s", m_headArmorFrameset, getHeadDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, headPosition));
  }

  auto frontArmDrawable = [&](String const& frameSet, String const& directives) -> Drawable {
    String image = strf("%s:%s%s", frameSet, frontHand.frontFrame, directives);
    Drawable frontArm = Drawable::makeImage(image, 1.0f / TilePixels, true, frontArmFrameOffset);
    frontArm.rotate(frontHand.angle, frontArmFrameOffset + m_frontArmRotationCenter);
    return frontArm;
  };

  if (frontHand.holdingItem && !dance.isValid()) {
    auto drawItem = [&]() {
      for (size_t i = 0; i < frontHand.itemDrawables.size(); i++) {
        Drawable frontHandItem = frontHand.itemDrawables[i];
        frontHandItem.translate(m_frontHandPosition + frontArmFrameOffset);
        frontHandItem.rotate(frontHand.itemAngle, frontArmFrameOffset + m_frontArmRotationCenter);
        addDrawable(frontHandItem);
      }
    };
    if (!frontHand.outsideOfHand)
      drawItem();
    if (!m_frontArmFrameset.empty() && !m_bodyHidden)
      addDrawable(frontArmDrawable(m_frontArmFrameset, getBodyDirectives()), m_bodyFullbright);
    if (!m_frontSleeveFrameset.empty())
      addDrawable(frontArmDrawable(m_frontSleeveFrameset, getChestDirectives()));
    if (frontHand.outsideOfHand)
      drawItem();
  } else {
    if (!m_frontArmFrameset.empty() && !m_bodyHidden) {
      String image;
      Vec2F position;
      if (dance.isValid() && danceStep->frontArmFrame) {
        image = strf("%s:%s%s", m_frontArmFrameset, *danceStep->frontArmFrame, getBodyDirectives());
        position = danceStep->frontArmOffset / TilePixels;
      } else if (m_state == Idle) {
        image = strf("%s:%s%s", m_frontArmFrameset, m_identity.personality.armIdle, getBodyDirectives());
        position = m_identity.personality.armOffset / TilePixels;
      } else
        image = strf("%s:%s.%s%s", m_frontArmFrameset, frameBase(m_state), armStateSeq, getBodyDirectives());
      auto drawable = Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, position);
      if (dance.isValid())
        drawable.rotate(danceStep->frontArmRotation);
      addDrawable(drawable, m_bodyFullbright);
    }

    if (!m_frontSleeveFrameset.empty()) {
      String image;
      Vec2F position;
      if (dance.isValid() && danceStep->frontArmFrame) {
        image = strf("%s:%s%s", m_frontSleeveFrameset, *danceStep->frontArmFrame, getChestDirectives());
        position = danceStep->frontArmOffset / TilePixels;
      } else if (m_state == Idle) {
        image = strf("%s:%s%s", m_frontSleeveFrameset, m_identity.personality.armIdle, getChestDirectives());
        position = m_identity.personality.armOffset / TilePixels;
      } else
        image = strf("%s:%s.%s%s", m_frontSleeveFrameset, frameBase(m_state), armStateSeq, getChestDirectives());
      auto drawable = Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, position);
      if (dance.isValid())
        drawable.rotate(danceStep->frontArmRotation);
      addDrawable(drawable);
    }
  }

  if (m_drawVaporTrail) {
    auto image = strf("%s:%d",
        m_vaporTrailFrameset,
        m_timing.genericSeq(m_animationTimer, m_vaporTrailCycle, m_vaporTrailFrames, true));
    addDrawable(Drawable::makeImage(std::move(image), 1.0f / TilePixels, true, {}));
  }

  if (m_primaryHand.nonRotatedDrawables.size())
    drawables.insertAllAt(0, m_primaryHand.nonRotatedDrawables);
  if (m_altHand.nonRotatedDrawables.size())
    drawables.insertAllAt(0, m_altHand.nonRotatedDrawables);

  Drawable::rotateAll(drawables, m_rotation);
  Drawable::translateAll(drawables, m_globalOffset);
  Drawable::rebaseAll(drawables);

  return drawables;
}

List<Drawable> Humanoid::renderPortrait(PortraitMode mode) const {
  List<Drawable> drawables;
  int emoteStateSeq = m_timing.emoteStateSeq(m_emoteAnimationTimer, m_emoteState);

  auto addDrawable = [&](Drawable drawable) {
    if (mode != PortraitMode::Full && mode != PortraitMode::FullNeutral
      && mode != PortraitMode::FullNude && mode != PortraitMode::FullNeutralNude) {
      // TODO: make this configurable
      drawable.imagePart().addDirectives("addmask=/humanoid/portraitMask.png;0;0", false);
    }
    drawables.append(std::move(drawable));
  };

  bool dressed = !(mode == PortraitMode::FullNude || mode == PortraitMode::FullNeutralNude);

  auto helmetMaskDirective = dressed ? getHelmetMaskDirectives() : "";

  auto personality = m_identity.personality;
  if (mode == PortraitMode::FullNeutral || mode == PortraitMode::FullNeutralNude)
    personality = Root::singleton().speciesDatabase()->species(m_identity.species)->personalities()[0];

  if (mode != PortraitMode::Head) {
    if (!m_backArmFrameset.empty()) {
      String image = strf("%s:%s%s", m_backArmFrameset, personality.armIdle, getBodyDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.armOffset));
    }
    if (dressed && !m_backSleeveFrameset.empty()) {
      String image = strf("%s:%s%s", m_backSleeveFrameset, personality.armIdle, getChestDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.armOffset));
    }
    if (mode != PortraitMode::Bust) {
      if (dressed && !m_backArmorFrameset.empty()) {
        String image = strf("%s:%s%s", m_backArmorFrameset, personality.idle, getBackDirectives());
        addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, {}));
      }
    }
  }

  if (!m_headFrameset.empty()) {
    String image = strf("%s:normal%s", m_headFrameset, getBodyDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (!m_emoteFrameset.empty()) {
    String image =
        strf("%s:%s.%s%s", m_emoteFrameset, emoteFrameBase(m_emoteState), emoteStateSeq, getEmoteDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (!m_hairFrameset.empty()) {
    String image = strf("%s:normal%s", m_hairFrameset, getHairDirectives() + helmetMaskDirective);
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (!m_bodyFrameset.empty()) {
    String image = strf("%s:%s%s", m_bodyFrameset, personality.idle, getBodyDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, {}));
  }

  if (mode != PortraitMode::Head) {
    if (dressed && !m_legsArmorFrameset.empty()) {
      String image = strf("%s:%s%s", m_legsArmorFrameset, personality.idle, getLegsDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, {}));
    }

    if (dressed && !m_chestArmorFrameset.empty()) {
      String image = strf("%s:%s%s", m_chestArmorFrameset, personality.idle, getChestDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, {}));
    }
  }

  if (!m_facialHairFrameset.empty()) {
    String image = strf("%s:normal%s", m_facialHairFrameset, getFacialHairDirectives() + helmetMaskDirective);
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (!m_facialMaskFrameset.empty()) {
    String image = strf("%s:normal%s", m_facialMaskFrameset, getFacialMaskDirectives() + helmetMaskDirective);
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (dressed && !m_headArmorFrameset.empty()) {
    String image = strf("%s:normal%s", m_headArmorFrameset, getHeadDirectives());
    addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.headOffset));
  }

  if (mode != PortraitMode::Head) {
    if (!m_frontArmFrameset.empty()) {
      String image = strf("%s:%s%s", m_frontArmFrameset, personality.armIdle, getBodyDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.armOffset));
    }

    if (dressed && !m_frontSleeveFrameset.empty()) {
      String image = strf("%s:%s%s", m_frontSleeveFrameset, personality.armIdle, getChestDirectives());
      addDrawable(Drawable::makeImage(std::move(image), 1.0f, true, personality.armOffset));
    }
  }

  return drawables;
}

List<Drawable> Humanoid::renderSkull() const {
  return {Drawable::makeImage(
      Root::singleton().speciesDatabase()->species(m_identity.species)->skull(), 1.0f, true, Vec2F())};
}

List<Drawable> Humanoid::renderDummy(Gender gender, HeadArmor const* head, ChestArmor const* chest, LegsArmor const* legs, BackArmor const* back) {
  auto assets = Root::singleton().assets();
  Humanoid humanoid(assets->json("/humanoid.config"));

  humanoid.m_headFrameset = assets->json("/humanoid/any/dummy.config:head").toString();
  humanoid.m_bodyFrameset = assets->json("/humanoid/any/dummy.config:body").toString();
  humanoid.m_frontArmFrameset = assets->json("/humanoid/any/dummy.config:frontArm").toString();
  humanoid.m_backArmFrameset = assets->json("/humanoid/any/dummy.config:backArm").toString();
  humanoid.setFacingDirection(DirectionNames.getLeft(assets->json("/humanoid/any/dummy.config:direction").toString()));

  if (head) {
    humanoid.setHeadArmorFrameset(head->frameset(gender));
    humanoid.setHeadArmorDirectives(head->directives());
    humanoid.setHelmetMaskDirectives(head->maskDirectives());
  }

  if (chest) {
    humanoid.setBackSleeveFrameset(chest->backSleeveFrameset(gender));
    humanoid.setFrontSleeveFrameset(chest->frontSleeveFrameset(gender));
    humanoid.setChestArmorFrameset(chest->bodyFrameset(gender));
    humanoid.setChestArmorDirectives(chest->directives());
  }

  if (legs) {
    humanoid.setLegsArmorFrameset(legs->frameset(gender));
    humanoid.setLegsArmorDirectives(legs->directives());
  }

  if (back) {
    humanoid.setBackArmorFrameset(back->frameset(gender));
    humanoid.setBackArmorDirectives(back->directives());
  }

  auto drawables = humanoid.render();
  Drawable::scaleAll(drawables, TilePixels);

  return drawables;
}

Vec2F Humanoid::primaryHandPosition(Vec2F const& offset) const {
  return primaryArmPosition(m_facingDirection, m_primaryHand.angle, primaryHandOffset(m_facingDirection) + offset);
}

Vec2F Humanoid::altHandPosition(Vec2F const& offset) const {
  return altArmPosition(m_facingDirection, m_altHand.angle, altHandOffset(m_facingDirection) + offset);
}

Vec2F Humanoid::primaryArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const {
  float bobYOffset = getBobYOffset();

  if (m_primaryHand.holdingItem) {
    Vec2F rotationCenter;
    if (facingDirection == Direction::Left || m_twoHanded)
      rotationCenter = m_frontArmRotationCenter + Vec2F(0, bobYOffset);
    else
      rotationCenter = m_backArmRotationCenter + Vec2F(0, bobYOffset) + m_backArmOffset;

    Vec2F position = offset.rotate(armAngle) + rotationCenter;

    if (facingDirection == Direction::Left)
      position[0] *= -1;

    return position;
  } else {
    // TODO: non-aiming item holding not supported yet
    return Vec2F();
  }
}

Vec2F Humanoid::altArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const {
  float bobYOffset = getBobYOffset();

  if (m_altHand.holdingItem) {
    Vec2F rotationCenter;
    if (facingDirection == Direction::Right)
      rotationCenter = m_frontArmRotationCenter + Vec2F(0, bobYOffset);
    else
      rotationCenter = m_backArmRotationCenter + Vec2F(0, bobYOffset) + m_backArmOffset;

    Vec2F position = Vec2F(offset).rotate(armAngle) + rotationCenter;

    if (facingDirection == Direction::Left)
      position[0] *= -1;

    return position;
  } else {
    // TODO: non-aiming item holding not supported yet
    return Vec2F();
  }
}

Vec2F Humanoid::primaryHandOffset(Direction facingDirection) const {
  if (facingDirection == Direction::Left || m_twoHanded)
    return m_frontHandPosition - m_frontArmRotationCenter;
  else
    return m_frontHandPosition - m_backArmRotationCenter;
}

Vec2F Humanoid::altHandOffset(Direction facingDirection) const {
  if (facingDirection == Direction::Left || m_twoHanded)
    return m_frontHandPosition - m_backArmRotationCenter;
  else
    return m_frontHandPosition - m_frontArmRotationCenter;
}

String Humanoid::frameBase(State state) const {
  switch (state) {
    case State::Idle:
      return "idle";
    case State::Walk:
      return "walk";
    case State::Run:
      return "run";
    case State::Jump:
      return "jump";
    case State::Swim:
      return "swim";
    case State::SwimIdle:
      return "swimIdle";
    case State::Duck:
      return "duck";
    case State::Fall:
      return "fall";
    case State::Sit:
      return "sit";
    case State::Lay:
      return "lay";

    default:
      throw StarException(strf("No such state '%s'", state));
  }
}

String Humanoid::emoteFrameBase(HumanoidEmote state) const {
  switch (state) {
    case HumanoidEmote::Idle:
      return "idle";
    case HumanoidEmote::Blabbering:
      return "blabber";
    case HumanoidEmote::Shouting:
      return "shout";
    case HumanoidEmote::Happy:
      return "happy";
    case HumanoidEmote::Sad:
      return "sad";
    case HumanoidEmote::NEUTRAL:
      return "neutral";
    case HumanoidEmote::Laugh:
      return "laugh";
    case HumanoidEmote::Annoyed:
      return "annoyed";
    case HumanoidEmote::Oh:
      return "oh";
    case HumanoidEmote::OOOH:
      return "oooh";
    case HumanoidEmote::Blink:
      return "blink";
    case HumanoidEmote::Wink:
      return "wink";
    case HumanoidEmote::Eat:
      return "eat";
    case HumanoidEmote::Sleep:
      return "sleep";

    default:
      throw StarException(strf("No such emote state '%s'", HumanoidEmoteNames.getRight(state)));
  }
}

String Humanoid::getHeadFromIdentity() const {
  return strf("/humanoid/%s/%shead.png",
      m_identity.imagePath ? *m_identity.imagePath : m_identity.species,
      GenderNames.getRight(m_identity.gender));
}

String Humanoid::getBodyFromIdentity() const {
  return strf("/humanoid/%s/%sbody.png",
      m_identity.imagePath ? *m_identity.imagePath : m_identity.species,
      GenderNames.getRight(m_identity.gender));
}

String Humanoid::getFacialEmotesFromIdentity() const {
  return strf("/humanoid/%s/emote.png", m_identity.imagePath ? *m_identity.imagePath : m_identity.species);
}

String Humanoid::getHairFromIdentity() const {
  if (m_identity.hairType.empty())
    return "";
  return strf("/humanoid/%s/%s/%s.png",
      m_identity.imagePath ? *m_identity.imagePath : m_identity.species,
      m_identity.hairGroup,
      m_identity.hairType);
}

String Humanoid::getFacialHairFromIdentity() const {
  if (m_identity.facialHairType.empty())
    return "";
  return strf("/humanoid/%s/%s/%s.png",
      m_identity.imagePath ? *m_identity.imagePath : m_identity.species,
      m_identity.facialHairGroup,
      m_identity.facialHairType);
}

String Humanoid::getFacialMaskFromIdentity() const {
  if (m_identity.facialMaskType.empty())
    return "";
  return strf("/humanoid/%s/%s/%s.png",
      m_identity.imagePath ? *m_identity.imagePath : m_identity.species,
      m_identity.facialMaskGroup,
      m_identity.facialMaskType);
}

String Humanoid::getBackArmFromIdentity() const {
  return strf("/humanoid/%s/backarm.png", m_identity.imagePath ? *m_identity.imagePath : m_identity.species);
}

String Humanoid::getFrontArmFromIdentity() const {
  return strf("/humanoid/%s/frontarm.png", m_identity.imagePath ? *m_identity.imagePath : m_identity.species);
}

String Humanoid::getVaporTrailFrameset() const {
  return "/humanoid/any/flames.png";
}

String Humanoid::getBodyDirectives() const {
  return m_identity.bodyDirectives;
}

String Humanoid::getHairDirectives() const {
  return m_identity.hairDirectives;
}

String Humanoid::getEmoteDirectives() const {
  return m_identity.emoteDirectives;
}

String Humanoid::getFacialHairDirectives() const {
  return m_identity.facialHairDirectives;
}

String Humanoid::getFacialMaskDirectives() const {
  return m_identity.facialMaskDirectives;
}

String Humanoid::getHelmetMaskDirectives() const {
  return m_helmetMaskDirectives;
}

String Humanoid::getHeadDirectives() const {
  return m_headArmorDirectives;
}

String Humanoid::getChestDirectives() const {
  return m_chestArmorDirectives;
}

String Humanoid::getLegsDirectives() const {
  return m_legsArmorDirectives;
}

String Humanoid::getBackDirectives() const {
  return m_backArmorDirectives;
}

int Humanoid::getArmStateSequence() const {
  int stateSeq = m_timing.stateSeq(m_animationTimer, m_state);

  int armStateSeq;
  if (m_state == Walk) {
    armStateSeq = m_movingBackwards ? m_armWalkSeq.at(m_armWalkSeq.size() - stateSeq) : m_armWalkSeq.at(stateSeq - 1);
  } else if (m_state == Run) {
    armStateSeq = m_movingBackwards ? m_armRunSeq.at(m_armRunSeq.size() - stateSeq) : m_armRunSeq.at(stateSeq - 1);
  } else {
    armStateSeq = stateSeq;
  }

  return armStateSeq;
}

int Humanoid::getBodyStateSequence() const {
  int stateSeq = m_timing.stateSeq(m_animationTimer, m_state);

  int bodyStateSeq;
  if (m_movingBackwards && (m_state == Walk || m_state == Run))
    bodyStateSeq = m_timing.stateFrames[m_state] - stateSeq + 1;
  else
    bodyStateSeq = stateSeq;

  return bodyStateSeq;
}

Maybe<DancePtr> Humanoid::getDance() const {
  if (m_dance.isNothing())
    return {};

  auto danceDatabase = Root::singleton().danceDatabase();
  return danceDatabase->getDance(*m_dance);
}

float Humanoid::getBobYOffset() const {
  int bodyStateSeq = getBodyStateSequence();
  float bobYOffset = 0.0f;
  if (m_state == Run) {
    bobYOffset = m_runFallOffset + m_runBob.at(bodyStateSeq - 1);
  } else if (m_state == Fall) {
    bobYOffset = m_runFallOffset;
  } else if (m_state == Jump) {
    bobYOffset = m_jumpBob;
  } else if (m_state == Walk) {
    bobYOffset = m_walkBob.at(bodyStateSeq - 1);
  } else if (m_state == Swim) {
    bobYOffset = m_swimBob.at(bodyStateSeq - 1);
  } else if (m_state == Duck) {
    bobYOffset = m_duckOffset;
  } else if (m_state == Sit) {
    bobYOffset = m_sitOffset;
  } else if (m_state == Lay) {
    bobYOffset = m_layOffset;
  }
  return bobYOffset;
}

Vec2F Humanoid::armAdjustment() const {
  return Vec2F(0, getBobYOffset());
}

Vec2F Humanoid::mouthOffset(bool ignoreAdjustments) const {
  if (ignoreAdjustments) {
    return (m_mouthOffset).rotate(m_rotation);
  } else {
    Vec2F headPosition(0, getBobYOffset());
    if (m_state == Idle)
      headPosition += m_identity.personality.headOffset / TilePixels;
    else if (m_state == Run)
      headPosition += m_headRunOffset;
    else if (m_state == Swim || m_state == SwimIdle)
      headPosition += m_headSwimOffset;
    else if (m_state == Duck)
      headPosition += m_headDuckOffset;
    else if (m_state == Sit)
      headPosition += m_headSitOffset;
    else if (m_state == Lay)
      headPosition += m_headLayOffset;

    return (m_mouthOffset + headPosition).rotate(m_rotation);
  }
}

Vec2F Humanoid::feetOffset() const {
  return m_feetOffset.rotate(m_rotation);
}

Vec2F Humanoid::headArmorOffset() const {
  Vec2F headPosition(0, getBobYOffset());
  if (m_state == Idle)
    headPosition += m_identity.personality.headOffset / TilePixels;
  else if (m_state == Run)
    headPosition += m_headRunOffset;
  else if (m_state == Swim || m_state == SwimIdle)
    headPosition += m_headSwimOffset;
  else if (m_state == Duck)
    headPosition += m_headDuckOffset;
  else if (m_state == Sit)
    headPosition += m_headSitOffset;
  else if (m_state == Lay)
    headPosition += m_headLayOffset;

  return (m_headArmorOffset + headPosition).rotate(m_rotation);
}

Vec2F Humanoid::chestArmorOffset() const {
  Vec2F position(0, getBobYOffset());
  return (m_chestArmorOffset + position).rotate(m_rotation);
}

Vec2F Humanoid::legsArmorOffset() const {
  return m_legsArmorOffset.rotate(m_rotation);
}

Vec2F Humanoid::backArmorOffset() const {
  Vec2F position(0, getBobYOffset());
  return (m_backArmorOffset + position).rotate(m_rotation);
}

String Humanoid::defaultDeathParticles() const {
  return m_defaultDeathParticles;
}

List<Particle> Humanoid::particles(String const& name) const {
  auto particleDatabase = Root::singleton().particleDatabase();
  List<Particle> res;
  Json particles = m_particleEmitters.get(name).get("particles", {});
  for (auto particle : particles.toArray()) {
    auto particleSpec = particle.get("particle", {});
    res.push_back(particleDatabase->particle(particleSpec));
  }

  return res;
}

Json const& Humanoid::defaultMovementParameters() const {
  return m_defaultMovementParameters;
}

}
