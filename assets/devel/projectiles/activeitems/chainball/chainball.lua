require "/scripts/vec2.lua"
require "/scripts/util.lua"
require "/scripts/projectiles/orbit.lua"

function init()
  self.ownerId = projectile.sourceEntity()

  self.orbitPosition = world.entityPosition(self.ownerId)
  self.orbitDistance = config.getParameter("orbitDistance")
  self.orbitSpeed = config.getParameter("orbitSpeed")
  self.orbitParameters = config.getParameter("orbitParameters")
  mcontroller.applyParameters(self.orbitParameters)

  self.landed = false
  self.released = false
  self.releaseAngleTolerance = 0.2
  self.releaseParameters = config.getParameter("releaseParameters")

  self.pickupDistance = config.getParameter("pickupDistance")
end

function update(dt)
  if self.ownerId and world.entityExists(self.ownerId) then
    if not self.released then
      -- if not self.orbitParameters.collisionEnabled == false and mcontroller.isColliding() then
      -- if self.releaseAngle then
      --   sb.logInfo("angle %s was within %s of %s",
      --       mcontroller.rotation(),
      --       math.abs(util.angleDiff(mcontroller.rotation(), self.releaseAngle)),
      --       self.releaseAngle)
      -- end

      if self.releaseAngle and math.abs(util.angleDiff(mcontroller.rotation(), self.releaseAngle)) < self.releaseAngleTolerance then
        mcontroller.setVelocity(vec2.withAngle(self.releaseAngle, vec2.mag(mcontroller.velocity())))
        release()
      else
        orbit(self.orbitPosition, self.orbitDistance, self.orbitSpeed)
      end
    elseif not self.landed then
      self.landed = mcontroller.onGround() and vec2.mag(mcontroller.velocity()) < 0.2
    end

  else
    projectile.die()
  end
end

function updateOrbit(orbitPosition, orbitSpeed)
  self.orbitPosition = orbitPosition
  self.orbitSpeed = orbitSpeed
  projectile.setTimeToLive(5.0)
end

-- function holdChain()
--   local holdVector = world.distance(self.orbitPosition, mcontroller.position())
--   local orbitDistance = vec2.mag(holdVector)
--   sb.logInfo("Holding chain! length is %s, target length %s", orbitDistance, self.orbitDistance)
--   mcontroller.setVelocity(vec2.mul(vec2.norm(mcontroller.velocity()), self.baseSpeed))
--   if orbitDistance >= self.orbitDistance then
--     local holdAngle = vec2.angle(holdVector)
--     local holdVelocity = 20 * (orbitDistance - self.orbitDistance)
--     sb.logInfo("Holding at angle %s with target velocity %s", holdAngle, holdVelocity)
--     mcontroller.approachVelocityAlongAngle(holdAngle, holdVelocity, 1000, true)
--   end
--   projectile.setTimeToLive(2.0)
-- end

function pullChain(position, velocity, controlForce)
  local pullVector = world.distance(position, mcontroller.position())
  if vec2.mag(pullVector) < self.pickupDistance then
    kill()
  else
    local pullAngle = vec2.angle(pullVector)
    mcontroller.approachVelocityAlongAngle(pullAngle, velocity, controlForce, true)
    projectile.setTimeToLive(2.0)
  end
end

function speed()
  return vec2.mag(mcontroller.velocity())
end

function requestRelease(releaseAngle)
  self.releaseAngle = releaseAngle
end

function release()
  self.released = true
  mcontroller.applyParameters(self.releaseParameters)
end

function released()
  return self.released
end

function landed()
  return self.landed
end

function kill()
  projectile.die()
end
