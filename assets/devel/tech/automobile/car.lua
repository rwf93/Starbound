function init()
  self.specialLast = false
  self.active = false
  tech.setVisible(false)

  self.honkTimer = 0
  self.energyCostPerSecond = config.getParameter("energyCostPerSecond")
  self.carCustomMovementParameters = config.getParameter("carCustomMovementParameters")
  self.parentOffset = config.getParameter("parentOffset")

  self.collisionPoly = mcontroller.baseParameters().standingPoly
end

function uninit()
  if self.active then
    local transformPosition = transformPoly(self.collisionPoly)
    if transformPosition then
      mcontroller.setPosition(transformPosition)
    end
    deactivate()
  end
end

function update(args)
  local specialActivated = not self.specialLast and args.moves["special1"]
  self.specialLast = args.moves["special1"]

  if not self.active and not tech.parentLounging() and specialActivated then
    local transformPosition = transformPoly(self.carCustomMovementParameters.standingPoly)
    if transformPosition and status.overConsumeResource("energy", self.energyCostPerSecond * args.dt) then
      mcontroller.setPosition(transformPosition)
      activate()
    else
      -- Make some kind of error noise
    end
  elseif self.active and (specialActivated or not status.overConsumeResource("energy", self.energyCostPerSecond * args.dt)) then
    local transformPosition = transformPoly(self.collisionPoly)
    if transformPosition then
      mcontroller.setPosition(transformPosition)
    end
    deactivate()
  end

  if self.active then
    local diff = world.distance(tech.aimPosition(), mcontroller.position())
    local aimAngle = math.atan(diff[2], diff[1])
    local flip = aimAngle > math.pi / 2 or aimAngle < -math.pi / 2

    mcontroller.controlParameters(self.carCustomMovementParameters)

    if flip then
      animator.setFlipped(true)
      tech.setParentOffset({-self.parentOffset[1], self.parentOffset[2]})
      mcontroller.controlFace(-1)
    else
      animator.setFlipped(false)
      tech.setParentOffset(self.parentOffset)
      mcontroller.controlFace(1)
    end

    if not mcontroller.onGround() then
      if mcontroller.velocity()[2] > 0 then
        animator.setAnimationState("movement", "jump")
      else
        animator.setAnimationState("movement", "fall")
      end
    elseif mcontroller.walking() or mcontroller.running() then
      if flip and mcontroller.facingDirection() == 1 or not flip and mcontroller.facingDirection() == -1 then
        animator.setAnimationState("movement", "driveReverse")
      else
        animator.setAnimationState("movement", "driveForward")
      end
    else
      animator.setAnimationState("movement", "idle")
    end

    if args.moves["primaryFire"] and self.honkTimer <= 0 then
      animator.playSound("carHorn")
      self.honkTimer = config.getParameter("honkTime")
    end

    if self.honkTimer > 0 then self.honkTimer = self.honkTimer - args.dt end
  end
end

function activate()
  animator.burstParticleEmitter("carActivateParticles")
  tech.setVisible(true)
  tech.setParentState("sit")
  tech.setToolUsageSuppressed(true)
  self.active = true
end

function deactivate()
  animator.burstParticleEmitter("carDeactivateParticles")
  tech.setVisible(false)
  tech.setParentState()
  tech.setToolUsageSuppressed(false)
  tech.setParentOffset({0, 0})
  self.active = false
end

function transformPoly(toPoly)
  local position = mcontroller.position()
  local yAdjust = collisionBottom(mcontroller.collisionPoly()) - collisionBottom(toPoly)
  return world.resolvePolyCollision(toPoly, {position[1], position[2] + yAdjust}, 1)
end

function collisionBottom(collisionPoly)
  local lowest = 0
  for _,point in pairs(collisionPoly) do
    if point[2] < lowest then
      lowest = point[2]
    end
  end
  return lowest
end
