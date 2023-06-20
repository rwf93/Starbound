require "/scripts/vec2.lua"

function init()
  -- sb.logInfo("Initializing metagun")

  self.recoil = 0
  self.recoilRate = 0

  self.fireOffset = config.getParameter("fireOffset")
  updateAim()

  self.active = false
  storage.fireTimer = storage.fireTimer or 0

  animator.setPartTag("muzzleFlash", "variant", "1")
end

function uninit()
  -- sb.logInfo("Uninitializing metagun")
end

function update(dt, fireMode, shiftHeld)
  -- sb.logInfo("Updating metagun with fireMode %s and shiftHeld %s", fireMode, shiftHeld)

  updateAim()

  storage.fireTimer = math.max(storage.fireTimer - dt, 0)

  if self.active then
    self.recoilRate = 0
  else
    self.recoilRate = math.max(1, self.recoilRate + (10 * dt))
  end
  self.recoil = math.max(self.recoil - dt * self.recoilRate, 0)

  if self.active and storage.fireTimer <= 0 then
    self.recoil = math.pi/2 - self.aimAngle
    activeItem.setArmAngle(math.pi/2)
    if animator.animationState("firing") == "off" then
      animator.setAnimationState("firing", "fire")
    end
    animator.setPartTag("muzzleFlash", "variant", math.random(1, 3))
    storage.fireTimer = config.getParameter("fireTime", 1.0)

  end

  self.active = false
end

function activate(fireMode, shiftHeld)
  -- sb.logInfo("Activating metagun with fireMode %s and shiftHeld %s", fireMode, shiftHeld)

  self.active = true
end

function updateAim()
  self.aimAngle, self.aimDirection = activeItem.aimAngleAndDirection(self.fireOffset[2], activeItem.ownerAimPosition())
  self.aimAngle = self.aimAngle + self.recoil
  activeItem.setArmAngle(self.aimAngle)
  activeItem.setFacingDirection(self.aimDirection)
end

function firePosition()
  return vec2.add(mcontroller.position(), activeItem.handPosition(self.fireOffset))
end

function aimVector()
  local aimVector = vec2.rotate({1, 0}, self.aimAngle + sb.nrand(config.getParameter("inaccuracy", 0), 0))
  aimVector[1] = aimVector[1] * self.aimDirection
  return aimVector
end

function holdingItem()
  return true
end

function recoil()
  return false
end

function outsideOfHand()
  return false
end
