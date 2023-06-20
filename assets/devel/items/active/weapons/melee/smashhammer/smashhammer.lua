function init()
  self.rotation = 0
  self.rotationRate = 20.0
  self.rotationRange = {-0.4, math.pi + 0.2}
end

function update(dt, fireMode, shiftHeld)
  self.rotation = self.rotation + self.rotationRate * dt
  if self.rotation < self.rotationRange[1] or self.rotation > self.rotationRange[2] then
    self.rotationRate = -self.rotationRate
    self.rotation = self.rotation + self.rotationRate * dt * 2
  end
  local damageArea = animator.partPoly("hammer", "damageArea")
  if damageArea then
    activeItem.setDamageSources({{
        poly = damageArea,
        damage = 50,
        sourceEntity = activeItem.ownerEntityId(),
        team = activeItem.ownerTeam(),
        knockback = 50,
        rayCheck = true
      }})
  end
  activeItem.setArmAngle(self.rotation)
  activeItem.setFacingDirection(table.pack(activeItem.aimAngleAndDirection(-1, activeItem.ownerAimPosition()))[2])
end
