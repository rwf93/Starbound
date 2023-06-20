require "/scripts/vec2.lua"
require "/scripts/util.lua"

function init()
  self.fireOffset = config.getParameter("fireOffset", {0,0})
  self.fireDirection = config.getParameter("fireDirection")
  self.stances = config.getParameter("stances")
  self.projectileType = config.getParameter("projectileType")
  self.projectileParameters = config.getParameter("projectileParameters")
  self.returnSpeed = config.getParameter("returnSpeed", 0)
  self.returnDistance = config.getParameter("returnDistance", 1)

  self.swingSpeed = {30, 60}
  self.swingupTime = 1.5
  self.swingupTimer = 0
  self.swingRadius = 1.5

  self.releaseAngle = 0.15

  self.pullSpeed = 50
  self.pullForce = 500
  self.maxChainLength = 15
  self.currentChainLength = 0

  self.previousFireMode = "none"
  self.ropeSegments = {}

  self.ropeOffset = config.getParameter("ropeOffset", self.fireOffset)

  self.aimAngle = 0
  setStance("idle")
end

function update(dt, fireMode, shiftHeld)
  self.stanceTimer = math.max(self.stanceTimer - dt, 0)

  checkProjectile()

  if self.stanceName == "idle" then
    if fireMode == "primary" and self.previousFireMode ~= "primary" then
      fire()
    end
  elseif self.stanceName == "swingup" then
    if fireMode == "primary" then
      self.swingupTimer = self.swingupTimer + dt
      local swingSpeedRatio = math.min(1, self.swingupTimer / self.swingupTime)
      world.callScriptedEntity(self.projectileId, "updateOrbit", chainSourcePosition(), self.aimDirection * util.lerp(swingSpeedRatio, self.swingSpeed[1], self.swingSpeed[2]))
    elseif self.previousFireMode == "primary" then
      -- local releaseAngle = activeItem.aimAngleAndDirection(0, activeItem.ownerAimPosition())
      world.callScriptedEntity(self.projectileId, "requestRelease", self.aimDirection > 0 and self.releaseAngle or math.pi - self.releaseAngle)
      self.currentChainLength = self.maxChainLength
    end
  elseif self.stanceName == "release" then
    if fireMode == "primary" or world.callScriptedEntity(self.projectileId, "landed") or chainLength() > self.currentChainLength then
      setStance("retrieve")
    end
  elseif self.stanceName == "retrieve" then
    self.currentChainLength = math.min(self.currentChainLength, chainLength())
    if fireMode == "primary" and mcontroller.onGround() then
      mcontroller.controlModifiers({movementSuppressed=true})
      world.callScriptedEntity(self.projectileId, "pullChain", chainSourcePosition(), self.pullSpeed, self.pullForce)
    elseif chainLength() > self.currentChainLength + 0.1 then
      local pullVector = world.distance(world.entityPosition(self.projectileId), chainSourcePosition())
      local pullAngle = vec2.angle(pullVector)
      mcontroller.controlApproachVelocityAlongAngle(pullAngle, world.callScriptedEntity(self.projectileId, "speed"), 5000, true)
      if mcontroller.onGround() then
        world.callScriptedEntity(self.projectileId, "pullChain", chainSourcePosition(), 3, 500)
      end
    end
  end

  self.previousFireMode = fireMode

  updateAim(self.stance.allowRotate, self.stance.allowFlip)
  buildRopeSegments()
end

function uninit()
  if self.projectileId and world.entityExists(self.projectileId) then
    world.callScriptedEntity(self.projectileId, "kill")
  end
end

function fire()
  self.released = false
  self.projectileParameters.orbitDistance = self.swingRadius
  self.projectileParameters.orbitSpeed = self.aimDirection > 0 and self.swingSpeed[1] or -self.swingSpeed[1]
  self.projectileParameters.pickupDistance = 2.0
  local projectileId = world.spawnProjectile(
      self.projectileType,
      firePosition(),
      activeItem.ownerEntityId(),
      self.fireDirection,
      false,
      self.projectileParameters
    )
  if projectileId then
    self.projectileId = projectileId
    setStance("swingup")
    self.swingupTimer = 0
    animator.playSound("fire")
  end
end

function release()
  setStance("release")
  world.callScriptedEntity(self.projectileId, "release")
end

function setStance(stanceName)
  self.stanceName = stanceName
  self.stance = self.stances[stanceName]
  self.stanceTimer = self.stance.duration or 0
  animator.setAnimationState("weapon", stanceName == "idle" and "full" or "empty")
  animator.rotateGroup("weapon", util.toRadians(self.stance.weaponRotation))
  updateAim(self.stance.allowRotate, self.stance.allowFlip)
end

function resetChain()
  if self.projectileId then
    if world.entityExists(self.projectileId) then
      world.callScriptedEntity(self.projectileId, "kill")
    end
    self.projectileId = nil
  end
  setStance("idle")
end

function checkProjectile()
  if self.projectileId then
    if world.entityExists(self.projectileId) then
      if not self.released then
        self.released = world.callScriptedEntity(self.projectileId, "released")
        if self.released then
          setStance("release")
        end
      end
    else
      resetChain()
    end
  end
end

function buildRopeSegments()
  if self.projectileId and world.entityExists(self.projectileId) then
    local position = mcontroller.position()
    local handPosition = vec2.add(position, activeItem.handPosition(self.ropeOffset))
    activeItem.setScriptedAnimationParameter("p1", handPosition)
    activeItem.setScriptedAnimationParameter("p2", world.entityPosition(self.projectileId))
  else
    activeItem.setScriptedAnimationParameter("p1", nil)
    activeItem.setScriptedAnimationParameter("p2", nil)
  end
end

function updateAim(allowRotate, allowFlip)
  local aimAngle, aimDirection = activeItem.aimAngleAndDirection(self.fireOffset[2], activeItem.ownerAimPosition())

  if allowRotate then
    self.aimAngle = aimAngle
  end
  aimAngle = (self.aimAngle or 0) + util.toRadians(self.stance.armRotation)
  activeItem.setArmAngle(aimAngle)

  if allowFlip then
    self.aimDirection = aimDirection
  end
  activeItem.setFacingDirection((self.aimDirection or 0))
end

function aimVector()
  local aimVector = vec2.rotate({1, 0}, self.aimAngle)
  aimVector[1] = aimVector[1] * self.aimDirection
  return aimVector
end

function firePosition()
  local fireOffset = {self.fireOffset[1] * self.aimDirection, self.fireOffset[2]}
  return vec2.add(mcontroller.position(), activeItem.handPosition(fireOffset))
end

function chainSourcePosition()
  return vec2.add(mcontroller.position(), activeItem.handPosition(self.ropeOffset))
end

function chainLength()
  if self.projectileId and world.entityExists(self.projectileId) then
    return world.magnitude(world.entityPosition(self.projectileId), chainSourcePosition())
  else
    return 0
  end
end
