function init()
  self.mode = "none"
  self.timer = 0
  self.targetPosition = nil

  self.energyUsage = config.getParameter("energyUsage")
  self.blinkMode = config.getParameter("blinkMode")
  self.blinkOutTime = config.getParameter("blinkOutTime")
  self.blinkInTime = config.getParameter("blinkInTime")
end

function uninit()
  tech.setParentDirectives()
end

function update(args)
  local specialActivated = not self.specialLast and args.moves["special1"]
  self.specialLast = args.moves["special1"]

  if specialActivated and not tech.parentLounging() and self.mode == "none" then
    local blinkPosition = nil
    if self.blinkMode == "random" then
      local randomBlinkAvoidCollision = config.getParameter("randomBlinkAvoidCollision")
      local randomBlinkAvoidMidair = config.getParameter("randomBlinkAvoidMidair")
      local randomBlinkAvoidLiquid = config.getParameter("randomBlinkAvoidLiquid")

      blinkPosition =
        findRandomBlinkLocation(randomBlinkAvoidCollision, randomBlinkAvoidMidair, randomBlinkAvoidLiquid) or
        findRandomBlinkLocation(randomBlinkAvoidCollision, randomBlinkAvoidMidair, false) or
        findRandomBlinkLocation(randomBlinkAvoidCollision, false, false)
    elseif self.blinkMode == "cursor" then
      blinkPosition = blinkAdjust(tech.aimPosition(), true, true, false, false)
    elseif self.blinkMode == "cursorPenetrate" then
      blinkPosition = blinkAdjust(tech.aimPosition(), false, true, false, false)
    end

    if blinkPosition and status.overConsumeResource("energy", self.energyUsage) then
      self.targetPosition = blinkPosition
      self.mode = "start"
    else
      -- Make some kind of error noise
    end
  end

  if self.mode == "start" then
    mcontroller.setVelocity({0, 0})
    self.mode = "out"
    self.timer = 0
    animator.playSound("activate")
  elseif self.mode == "out" then
    tech.setParentDirectives("?multiply=00000000")
    animator.setAnimationState("blinking", "out")
    mcontroller.setVelocity({0, 0})
    self.timer = self.timer + args.dt

    if self.timer > self.blinkOutTime then
      mcontroller.setPosition(self.targetPosition)
      self.mode = "in"
      self.timer = 0
    end
  elseif self.mode == "in" then
    tech.setParentDirectives()
    animator.setAnimationState("blinking", "in")
    mcontroller.setVelocity({0, 0})
    self.timer = self.timer + args.dt

    if self.timer > self.blinkInTime then
      self.mode = "none"
    end
  end
end

function blinkAdjust(position, doPathCheck, doCollisionCheck, doLiquidCheck, doStandCheck)
  local blinkCollisionCheckDiameter = config.getParameter("blinkCollisionCheckDiameter")
  local blinkVerticalGroundCheck = config.getParameter("blinkVerticalGroundCheck")
  local blinkFootOffset = config.getParameter("blinkFootOffset")
  local blinkHeadOffset = config.getParameter("blinkHeadOffset")

  if doPathCheck then
    local collisionBlocks = world.collisionBlocksAlongLine(mcontroller.position(), position, {"Null", "Block", "Dynamic", "Slippery"}, 1)
    if #collisionBlocks ~= 0 then
      local diff = world.distance(position, mcontroller.position())
      diff[1] = diff[1] > 0 and 1 or -1
      diff[2] = diff[2] > 0 and 1 or -1

      position = {collisionBlocks[1][1] - math.min(diff[1], 0), collisionBlocks[1][2] - math.min(diff[2], 0)}
    end
  end

  if doCollisionCheck then
    local diff = world.distance(position, mcontroller.position())
    local collisionPoly = mcontroller.collisionPoly()

    --Add foot offset if there is ground
    if diff[2] < 0 then
      local groundBlocks = world.collisionBlocksAlongLine(position, {position[1], position[2] + blinkFootOffset}, {"Null", "Block", "Dynamic", "Slippery"}, 1)
      if #groundBlocks > 0 then
        position[2] = groundBlocks[1][2] + 1 - blinkFootOffset
      end
    end

    --Add head offset if there is ceiling
    if diff[2] > 0 then
      local ceilingBlocks = world.collisionBlocksAlongLine(position, {position[1], position[2] + blinkHeadOffset}, {"Null", "Block", "Dynamic", "Slippery"}, 1)
      if #ceilingBlocks > 0 then
        position[2] = ceilingBlocks[1][2] - blinkHeadOffset
      end
    end

    --Resolve position
    position = world.resolvePolyCollision(collisionPoly, position, blinkCollisionCheckDiameter)

    if not position or world.lineTileCollision(mcontroller.position(), position, {"Null", "Block", "Dynamic", "Slippery"}) then
      return nil
    end
  end

  if doStandCheck then
    local groundFound = false
    for i = 1, blinkVerticalGroundCheck * 2 do
      local checkPosition = {position[1], position[2] - i / 2}

      if world.pointTileCollision(checkPosition, {"Null", "Block", "Dynamic", "Platform"}) then
        groundFound = true
        position = {checkPosition[1], checkPosition[2] + 0.5 - blinkFootOffset}
        break
      end
    end

    if not groundFound then
      return nil
    end
  end

  if doLiquidCheck and (world.liquidAt(position) or world.liquidAt({position[1], position[2] + blinkFootOffset})) then
    return nil
  end

  return position
end

function findRandomBlinkLocation(doCollisionCheck, doLiquidCheck, doStandCheck)
  local randomBlinkTries = config.getParameter("randomBlinkTries")
  local randomBlinkDiameter = config.getParameter("randomBlinkDiameter")

  for i=1,randomBlinkTries do
    local position = mcontroller.position()
    position[1] = position[1] + (math.random() * 2 - 1) * randomBlinkDiameter
    position[2] = position[2] + (math.random() * 2 - 1) * randomBlinkDiameter

    local position = blinkAdjust(position, false, doCollisionCheck, doLiquidCheck, doStandCheck)
    if position then
      return position
    end
  end

  return nil
end
