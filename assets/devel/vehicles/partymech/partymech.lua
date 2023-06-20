require "/scripts/vec2.lua"
require "/scripts/util.lua"

function init()
  self.active = false
  self.fireTimer = 0
  animator.rotateGroup("guns", 0, true)
  self.level = config.getParameter("mechLevel", 6)
  self.groundFrames = 1
end

function update()
  if mcontroller.atWorldLimit() then
    vehicle.destroy()
    return
  end

  local mechHorizontalMovement = config.getParameter("mechHorizontalMovement")
  local mechJumpVelocity = config.getParameter("mechJumpVelocity")
  local offGroundFrames = config.getParameter("offGroundFrames")

  local mechCollisionPoly = mcontroller.collisionPoly()
  local position = mcontroller.position()

  local entityInSeat = vehicle.entityLoungingIn("seat")
  if entityInSeat then
    vehicle.setDamageTeam(world.entityDamageTeam(entityInSeat))
  else
    vehicle.setDamageTeam({type = "passive"})
  end

  local diff = world.distance(vehicle.aimPosition("seat"), mcontroller.position())
  local aimAngle = math.atan(diff[2], diff[1])
  local facingDirection = (aimAngle > math.pi / 2 or aimAngle < -math.pi / 2) and -1 or 1

  if facingDirection < 0 then
    animator.setFlipped(true)
  else
    animator.setFlipped(false)
  end

  local onGround = mcontroller.onGround()
  local movingDirection = 0

  if vehicle.controlHeld("seat", "left") and onGround then
    mcontroller.setXVelocity(-mechHorizontalMovement)
    movingDirection = -1
  end

  if vehicle.controlHeld("seat", "right") and onGround then
    mcontroller.setXVelocity(mechHorizontalMovement)
    movingDirection = 1
  end

  if vehicle.controlHeld("seat", "jump") and onGround then
    mcontroller.setXVelocity(mechJumpVelocity[1] * movingDirection)
    mcontroller.setYVelocity(mechJumpVelocity[2])
    animator.setAnimationState("movement", "jump")
    self.groundFrames = 0
  end

  if onGround then
    self.groundFrames = offGroundFrames
  else
    self.groundFrames = self.groundFrames - 1
  end

  if self.groundFrames <= 0 then
    if mcontroller.velocity()[2] > 0 then
      animator.setAnimationState("movement", "jump")
    else
      animator.setAnimationState("movement", "fall")
    end
  elseif movingDirection ~= 0 then
    if facingDirection ~= movingDirection then
      animator.setAnimationState("movement", "backWalk")
    else
      animator.setAnimationState("movement", "walk")
    end
  elseif onGround then
    animator.setAnimationState("movement", "idle")
  end

  if vehicle.controlHeld("seat", "primaryFire") then
    -- TODO: party harder
  end
end
