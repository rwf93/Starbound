require "/vehicles/modularmech/armscripts/base.lua"

PlayerLauncher = MechArm:extend()

function PlayerLauncher:init()
  self.reloadTimer = 0
  self.bobLocked = true
  -- vehicle.setLoungeStatusEffects("z" .. self.armName .. "Seat", {"mechballistic"})
end

function PlayerLauncher:update(dt)
  if aimPosition then
    animator.rotateTransformationGroup(self.armName, self.aimAngle, self.shoulderOffset)

    self.reloadTimer = math.max(0, self.reloadTimer - dt)
    if self.fireTriggered and self.reloadTimer == 0 then
      self.reloadTimer = self.reloadTime
      animator.playSound(self.armName .. "Fire")
      animator.setAnimationState(self.armName, "winddown", true)
      vehicle.setLoungeStatusEffects("z" .. self.armName .. "Seat", {"mechballistic"})
    elseif self.reloadTimer > 0 then
      -- vehicle.setLoungeStatusEffects("z" .. self.armName .. "Seat", {})
      -- vehicle.setLoungeEnabled("z" .. self.armName .. "Seat", false)
    else
      vehicle.setLoungeStatusEffects("z" .. self.armName .. "Seat", {})
      vehicle.setLoungeEnabled("z" .. self.armName .. "Seat", true)
      animator.setAnimationState(self.armName, "rotate")

    end
  else
    vehicle.setLoungeStatusEffects("z" .. self.armName .. "Seat", {})
    vehicle.setLoungeEnabled("z" .. self.armName .. "Seat", false)
    animator.setAnimationState(self.armName, "idle")
  end
end
