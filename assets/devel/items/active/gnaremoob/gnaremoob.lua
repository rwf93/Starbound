require "/scripts/vec2.lua"

function init()
  self.aimAndDir = table.pack(activeItem.aimAngleAndDirection(-1, activeItem.ownerAimPosition()))
  idle()
end

function uninit()
  mcontroller.setRotation(0)
end

function update(dt, fireMode, shiftHeld)
  if self.state == "idle" then
    self.aimAndDir = table.pack(activeItem.aimAngleAndDirection(-1, activeItem.ownerAimPosition()))
    activeItem.setArmAngle(self.aimAndDir[1])
    activeItem.setFacingDirection(self.aimAndDir[2])
    if fireMode == "primary" then
      windup()
    end
  end

  if self.state == "windup" then
    self.stateTimer = math.max(0, self.stateTimer - dt)
    if self.stateTimer == 0 then
      flyout()
    end
  end

  if self.state == "flyout" then
    self.stateTimer = math.max(0, self.stateTimer - dt)
    if (self.stateTimer > 0 or not mcontroller.isColliding()) and vec2.mag(mcontroller.velocity()) > 0.1 then
      mcontroller.controlParameters({
          gravityEnabled=false,
          collisionPoly=config.getParameter("flyCollisionPoly"),
          standingPoly=config.getParameter("flyCollisionPoly"),
          crouchingPoly=config.getParameter("flyCollisionPoly")
        })
      mcontroller.controlModifiers({movementSuppressed=true})
      mcontroller.controlApproachVelocity({0, 0}, config.getParameter("controlForce"))
      self.rotation = self.rotation - config.getParameter("spinRate") * dt * self.aimAndDir[2]
      mcontroller.setRotation(self.rotation)
    else
      flyback()
    end
  end

  if self.state == "flyback" then
    if world.magnitude(mcontroller.position(), self.returnPosition) > config.getParameter("snapDistance") then
      mcontroller.controlParameters({gravityEnabled=false,collisionEnabled=false})
      mcontroller.controlModifiers({movementSuppressed=true})
      local toTargetVelocity = vec2.mul(vec2.norm(world.distance(self.returnPosition, mcontroller.position())), config.getParameter("flySpeed"))
      mcontroller.controlApproachVelocity(toTargetVelocity, config.getParameter("controlForce"))
      self.rotation = self.rotation - config.getParameter("spinRate") * dt * self.aimAndDir[2]
      mcontroller.setRotation(self.rotation)
    else
      mcontroller.setVelocity({0, 0})
      idle()
    end
  end
end

function idle()
  self.state = "idle"
  self.rotation = 0
  mcontroller.setRotation(self.rotation)
  activeItem.setTwoHandedGrip(false)
end

function windup()
  self.state = "windup"
  self.stateTimer = config.getParameter("windupTime")
  activeItem.setArmAngle(self.aimAndDir[1] + 1.2)
end

function flyout()
  self.state = "flyout"
  self.stateTimer = config.getParameter("minFlyTime")
  self.returnPosition = mcontroller.position()
  local toTargetVelocity = vec2.rotate({config.getParameter("flySpeed"), 0}, self.aimAndDir[1])
  toTargetVelocity[1] = toTargetVelocity[1] * self.aimAndDir[2]
  mcontroller.setVelocity(toTargetVelocity)
  activeItem.setTwoHandedGrip(true)
  activeItem.setArmAngle(0)
end

function flyback()
  self.state = "flyback"
  activeItem.setTwoHandedGrip(true)
  mcontroller.setVelocity(vec2.mul(mcontroller.velocity(), -1))
end
