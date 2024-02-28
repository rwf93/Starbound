#include "StarItemDrop.hpp"
#include "StarRandom.hpp"
#include "StarAssets.hpp"
#include "StarRoot.hpp"
#include "StarItemDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarEntityRendering.hpp"
#include "StarWorld.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {

ItemDropPtr ItemDrop::createRandomizedDrop(ItemPtr const& item, Vec2F const& position, bool eternal) {
  if (!item)
    return {};

  auto idconfig = Root::singleton().assets()->json("/itemdrop.config");

  ItemDropPtr itemDrop = make_shared<ItemDrop>(item);
  auto offset = Vec2F(idconfig.getFloat("randomizedDistance"), 0).rotate(Constants::pi * 2.0 * Random::randf());
  offset[1] = fabs(offset[1]);
  itemDrop->setPosition(position + offset / TilePixels);
  itemDrop->setVelocity(offset * idconfig.getFloat("randomizedSpeed"));
  itemDrop->setEternal(eternal);

  return itemDrop;
}

ItemDropPtr ItemDrop::createRandomizedDrop(ItemDescriptor const& descriptor, Vec2F const& position, bool eternal) {
  if (!descriptor || descriptor.isEmpty())
    return {};

  auto itemDatabase = Root::singleton().itemDatabase();
  auto itemDrop = createRandomizedDrop(itemDatabase->item(descriptor), position);
  itemDrop->setEternal(eternal);

  return itemDrop;
}

ItemDropPtr ItemDrop::throwDrop(ItemPtr const& item, Vec2F const& position, Vec2F const& direction, bool eternal) {
  if (!item)
    return {};

  auto idconfig = Root::singleton().assets()->json("/itemdrop.config");

  ItemDropPtr itemDrop = make_shared<ItemDrop>(item);
  itemDrop->setPosition(position);
  if (direction != Vec2F())
    itemDrop->setVelocity(vnorm(direction) * idconfig.getFloat("throwSpeed"));

  itemDrop->setEternal(eternal);
  itemDrop->setIntangibleTime(idconfig.getFloat("throwIntangibleTime"));

  return itemDrop;
}

ItemDropPtr ItemDrop::throwDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, Vec2F const& direction, bool eternal) {
  if (!itemDescriptor || itemDescriptor.isEmpty())
    return {};

  auto itemDatabase = Root::singleton().itemDatabase();
  auto itemDrop = throwDrop(itemDatabase->item(itemDescriptor), position, direction);
  itemDrop->setEternal(eternal);

  return itemDrop;
}

ItemDrop::ItemDrop(ItemPtr item)
  : ItemDrop() {
  m_item = std::move(item);

  updateCollisionPoly();

  m_owningEntity.set(NullEntityId);
  m_mode.set(Mode::Available);
  m_itemDescriptor.set(m_item->descriptor());
}

ItemDrop::ItemDrop(Json const& diskStore)
  : ItemDrop() {
  Root::singleton().itemDatabase()->diskLoad(diskStore.get("item"), m_item);
  m_movementController.setPosition(jsonToVec2F(diskStore.get("position")));
  m_mode.set(ModeNames.getLeft(diskStore.getString("mode")));
  m_eternal = diskStore.getBool("eternal");
  m_dropAge = EpochTimer(diskStore.get("dropAge"));
  m_ageItemsTimer = EpochTimer(diskStore.get("ageItemsTimer"));

  updateCollisionPoly();
  m_owningEntity.set(NullEntityId);
  m_itemDescriptor.set(m_item->descriptor());
}

ItemDrop::ItemDrop(ByteArray store)
  : ItemDrop() {
  DataStreamBuffer ds(std::move(store));

  Root::singleton().itemDatabase()->loadItem(ds.read<ItemDescriptor>(), m_item);
  ds.read(m_eternal);
  ds.read(m_dropAge);
  ds.read(m_intangibleTimer);

  updateCollisionPoly();
}

Json ItemDrop::diskStore() const {
  auto itemDatabase = Root::singleton().itemDatabase();
  return JsonObject{
    {"item", itemDatabase->diskStore(m_item)},
    {"position", jsonFromVec2F(m_movementController.position())},
    {"mode", ModeNames.getRight(m_mode.get())},
    {"eternal", m_eternal},
    {"dropAge", m_dropAge.toJson()},
    {"ageItemsTimer", m_ageItemsTimer.toJson()}
  };
}

ByteArray ItemDrop::netStore() const {
  DataStreamBuffer ds;

  ds.write(itemSafeDescriptor(m_item));
  ds.write(m_eternal);
  ds.write(m_dropAge);
  ds.write(m_intangibleTimer);

  return ds.takeData();
}

EntityType ItemDrop::entityType() const {
  return EntityType::ItemDrop;
}

void ItemDrop::init(World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);

  m_movementController.init(world);
}

void ItemDrop::uninit() {
  Entity::uninit();
  m_movementController.uninit();
}

String ItemDrop::description() const {
  return m_item->description();
}

pair<ByteArray, uint64_t> ItemDrop::writeNetState(uint64_t fromVersion) {
  return m_netGroup.writeNetState(fromVersion);
}

void ItemDrop::readNetState(ByteArray data, float interpolationTime) {
  m_netGroup.readNetState(std::move(data), interpolationTime);
}

void ItemDrop::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void ItemDrop::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

Vec2F ItemDrop::position() const {
  return m_movementController.position();
}

RectF ItemDrop::metaBoundBox() const {
  return m_boundBox;
}

bool ItemDrop::ephemeral() const {
  return true;
}

RectF ItemDrop::collisionArea() const {
  return m_boundBox;
}

void ItemDrop::update(uint64_t) {
  if (isMaster()) {
    if (m_owningEntity.get() != NullEntityId) {
      auto owningEntity = world()->entity(m_owningEntity.get());
      if (owningEntity) {
        Vec2F position = m_movementController.position();
        Vec2F diff = world()->geometry().diff(owningEntity->position(), position);
        float magnitude = diff.magnitude();
        m_movementController.approachVelocity(diff.normalized() * m_config.getFloat("velocity"), m_config.getFloat("velocityApproach"));
        if (magnitude < m_config.getFloat("pickupDistance"))
          m_mode.set(Mode::Dead);

      } else {
        // Our owning entity left, disappear quickly
        m_mode.set(Mode::Dead);
      }

      MovementParameters parameters;
      parameters.collisionEnabled = false;
      parameters.gravityEnabled = false;
      m_movementController.applyParameters(parameters);
    } else {
      // Rarely, check for other drops near us and combine with them if possible.
      if (canTake() && m_mode.get() == Mode::Available && Random::randf() < m_config.getFloat("combineChance")) {
        world()->findEntity(RectF::withCenter(position(), Vec2F::filled(m_config.getFloat("combineRadius"))), [&](EntityPtr const& entity) {
            if (auto closeDrop = as<ItemDrop>(entity)) {
              // Make sure not to try to merge with ourselves here.
              if (closeDrop.get() != this && closeDrop->canTake()
                  && vmag(position() - closeDrop->position()) < m_config.getFloat("combineRadius")) {
                if (m_item->couldStack(closeDrop->item()) == closeDrop->item()->count()) {
                  m_item->stackWith(closeDrop->take());
                  m_dropAge.setElapsedTime(min(m_dropAge.elapsedTime(), closeDrop->m_dropAge.elapsedTime()));

                  // Average the position and velocity of the drop we merged
                  // with
                  m_movementController.setPosition(m_movementController.position()
                      + world()->geometry().diff(closeDrop->position(), m_movementController.position()) / 2.0f);
                  m_movementController.setVelocity((m_movementController.velocity() + closeDrop->velocity()) / 2.0f);
                  return true;
                }
              }
            }
            return false;
          });
      }

      MovementParameters parameters;
      parameters.collisionEnabled = true;
      parameters.gravityEnabled = true;
      m_movementController.applyParameters(parameters);
    }

    m_movementController.tickMaster();

    m_intangibleTimer.tick(WorldTimestep);
    m_dropAge.update(world()->epochTime());
    m_ageItemsTimer.update(world()->epochTime());

    if ((m_mode.get() == Mode::Intangible || m_mode.get() == Mode::Available) && m_movementController.atWorldLimit())
      m_mode.set(Mode::Dead);
    if (m_mode.get() == Mode::Intangible && m_intangibleTimer.ready())
      m_mode.set(Mode::Available);
    if (!m_eternal && m_mode.get() == Mode::Available && m_dropAge.elapsedTime() > m_item->timeToLive())
      m_mode.set(Mode::Dead);
    if (m_mode.get() == Mode::Taken && m_dropAge.elapsedTime() > m_config.getFloat("afterTakenLife"))
      m_mode.set(Mode::Dead);

    if (m_mode.get() <= Mode::Available && m_ageItemsTimer.elapsedTime() > m_config.getDouble("ageItemsEvery", 10)) {
      Root::singleton().itemDatabase()->ageItem(m_item, m_ageItemsTimer.elapsedTime());
      m_itemDescriptor.set(m_item->descriptor());
      updateCollisionPoly();
      m_ageItemsTimer.setElapsedTime(0.0);
    }
  } else {
    m_netGroup.tickNetInterpolation(WorldTimestep);
    Root::singleton().itemDatabase()->loadItem(m_itemDescriptor.get(), m_item);
    m_movementController.tickSlave();
  }
}

bool ItemDrop::shouldDestroy() const {
  return m_mode.get() == Mode::Dead;
}

void ItemDrop::render(RenderCallback* renderCallback) {
  for (auto& drawable : m_item->dropDrawables()) {
    drawable.translate(position());
    renderCallback->addDrawable(std::move(drawable), RenderLayerItemDrop);
  }
}

ItemPtr ItemDrop::item() const {
  return m_item;
}

void ItemDrop::setEternal(bool eternal) {
  m_eternal = eternal;
}

void ItemDrop::setIntangibleTime(float intangibleTime) {
  m_intangibleTimer = GameTimer(intangibleTime);
  if (m_mode.get() == Mode::Available)
    m_mode.set(Mode::Intangible);
}

bool ItemDrop::canTake() const {
  return m_mode.get() == Mode::Available && m_owningEntity.get() == NullEntityId && !m_item->empty();
}

ItemPtr ItemDrop::takeBy(EntityId entityId) {
  if (canTake()) {
    m_owningEntity.set(entityId);
    m_dropAge.setElapsedTime(0.0);
    m_mode.set(Mode::Taken);
    setPersistent(false);

    return m_item->take();
  } else {
    return {};
  }
}

ItemPtr ItemDrop::take() {
  if (canTake()) {
    m_mode.set(Mode::Dead);
    return m_item->take();
  } else {
    return {};
  }
}

void ItemDrop::setPosition(Vec2F const& position) {
  m_movementController.setPosition(position);
}

Vec2F ItemDrop::velocity() const {
  return m_movementController.velocity();
}

void ItemDrop::setVelocity(Vec2F const& velocity) {
  m_movementController.setVelocity(velocity);
}

EnumMap<ItemDrop::Mode> const ItemDrop::ModeNames{{ItemDrop::Mode::Intangible, "Intangible"},
    {ItemDrop::Mode::Available, "Available"},
    {ItemDrop::Mode::Taken, "Taken"},
    {ItemDrop::Mode::Dead, "Dead"}};

ItemDrop::ItemDrop() {
  setPersistent(true);

  m_config = Root::singleton().assets()->json("/itemdrop.config");

  MovementParameters parameters = MovementParameters(m_config.get("movementSettings", JsonObject()));
  if (!parameters.physicsEffectCategories)
    parameters.physicsEffectCategories = StringSet({"itemdrop"});
  m_movementController.applyParameters(parameters);

  m_netGroup.addNetElement(&m_mode);
  m_netGroup.addNetElement(&m_owningEntity);
  m_netGroup.addNetElement(&m_movementController);
  m_netGroup.addNetElement(&m_itemDescriptor);

  m_eternal = false;
}

void ItemDrop::updateCollisionPoly() {
  m_boundBox = Drawable::boundBoxAll(m_item->dropDrawables(), true);
  m_boundBox.rangeSetIfEmpty(RectF{-0.5, -0.5, 0.5, 0.5});
  MovementParameters parameters;
  parameters.collisionPoly = PolyF(collisionArea());
  m_movementController.applyParameters(parameters);
}

}
