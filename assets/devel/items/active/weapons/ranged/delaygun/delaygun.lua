require "/scripts/vec2.lua"

function init()
  -- scale damage and calculate energy cost
  self.pType = config.getParameter("projectileType")
  self.pParams = config.getParameter("projectileParameters", {})
  if not self.pParams.power then
    local projectileConfig = root.projectileConfig(self.pType)
    self.pParams.power = projectileConfig.power
  end
  self.pParams.power = self.pParams.power * root.evalFunction("weaponDamageLevelMultiplier", config.getParameter("level", 1))
  self.energyPerShot = 3 * self.pParams.power

  self.fireOffset = config.getParameter("fireOffset")
  updateAim()

  storage.fireTimer = storage.fireTimer or 0
  self.recoilTimer = 0

  animator.setPartTag("muzzleFlash", "variant", "1")
  activeItem.setCursor("/cursors/reticle0.cursor")

  self.activeBullets = {}
end

function activate(fireMode, shiftHeld)
  if fireMode == "alt" then
    triggerBullets()
  end
end

function update(dt, fireMode, shiftHeld)
  updateAim()
  updateBullets()

  storage.fireTimer = math.max(storage.fireTimer - dt, 0)
  self.recoilTimer = math.max(self.recoilTimer - dt, 0)

  if fireMode == "primary"
      and storage.fireTimer == 0
      and not world.pointTileCollision(firePosition())
      and status.overConsumeResource("energy", self.energyPerShot) then

    storage.fireTimer = config.getParameter("fireTime", 1.0)
    fire()
  end

  activeItem.setRecoil(self.recoilTimer > 0)
  animator.setLightActive("muzzleFlash", self.recoilTimer > 0)
end

function fire()
  self.pParams.powerMultiplier = activeItem.ownerPowerMultiplier()
  local bulletId = world.spawnProjectile(
      self.pType,
      firePosition(),
      activeItem.ownerEntityId(),
      aimVector(),
      false,
      self.pParams
    )
  if bulletId then
    self.activeBullets[#self.activeBullets + 1] = bulletId
  end
  animator.setAnimationState("firing", "fire", true)
  animator.setPartTag("muzzleFlash", "variant", math.random(1, 3))
  animator.playSound("fire")
  self.recoilTimer = config.getParameter("recoilTime", 0.08)
end

function updateAim()
  self.aimAngle, self.aimDirection = activeItem.aimAngleAndDirection(config.getParameter("aimOffset"), activeItem.ownerAimPosition())
  activeItem.setArmAngle(self.aimAngle)
  activeItem.setFacingDirection(self.aimDirection)
end

function updateBullets()
  local newBullets = {}
  for i, bullet in ipairs(self.activeBullets) do
    if world.entityExists(bullet) then
      newBullets[#newBullets + 1] = bullet
    end
  end
  self.activeBullets = newBullets
end

function triggerBullets()
  for i, bullet in ipairs(self.activeBullets) do
    world.callScriptedEntity(bullet, "trigger")
  end
end

function firePosition()
  return vec2.add(mcontroller.position(), activeItem.handPosition(self.fireOffset))
end

function aimVector()
  local aimVector = vec2.rotate({1, 0}, self.aimAngle + sb.nrand(config.getParameter("inaccuracy", 0), 0))
  aimVector[1] = aimVector[1] * self.aimDirection
  return aimVector
end
