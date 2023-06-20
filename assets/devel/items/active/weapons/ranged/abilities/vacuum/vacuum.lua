require "/scripts/poly.lua"
require "/scripts/vec2.lua"

function setupAbility(altAbilityConfig)
  local vacuum = WeaponAbility:new()

  function vacuum:init()
    self.cooldownTimer = 0
    self:reset()
  end

  function vacuum:update(dt, fireMode, shiftHeld)
    WeaponAbility.update(self, dt, fireMode, shiftHeld)

    self.cooldownTimer = math.max(0, self.cooldownTimer - self.dt)

    if self.fireMode == "alt"
      and not self.active
      and self.cooldownTimer == 0
      and not status.resourceLocked("energy") then

      self:setState(self.fire)
    end
  end

  function vacuum:fire()
    self.weapon:setStance(self.stances.fire)
    -- self.weapon.aimAngle = 0
    -- self.weapon:updateAim()
    animator.setAnimationState("vacuum", "active")
    animator.playSound("vacuumStart")
    animator.playSound("vacuumLoop", -1)

    local vacuumPoint = {
      type = "RadialForceRegion",
      center = animator.partPoint("vacuumCone", "vacuumPoint"),
      targetRadialVelocity = self.pointSpeed,
      outerRadius = 1.5,
      innerRadius = 0.5,
      controlForce = self.pointForce
    }

    self.active = true

    while self.fireMode == "alt" and status.overConsumeResource("energy", self.energyUsage * self.dt) do
      mcontroller.controlModifiers({runningSuppressed = true})

      local forceVector = vec2.rotate(self.coneSpeed, self.weapon.aimAngle)
      forceVector[1] = forceVector[1] * self.weapon.aimDirection

      local vacuumConeTop = {
        type = "DirectionalForceRegion",
        polyRegion = animator.partPoly("vacuumCone", "vacuumPolyTop"),
        xTargetVelocity = forceVector[1],
        yTargetVelocity = -forceVector[2],
        controlForce = self.coneForce
      }
      local vacuumConeBottom = {
        type = "DirectionalForceRegion",
        polyRegion = animator.partPoly("vacuumCone", "vacuumPolyBottom"),
        xTargetVelocity = forceVector[1],
        yTargetVelocity = forceVector[2],
        controlForce = self.coneForce
      }
      activeItem.setItemForceRegions({vacuumConeTop, vacuumConeBottom, vacuumPoint})

      coroutine.yield()
    end

    activeItem.setItemForceRegions({})

    animator.setAnimationState("vacuum", "idle")
    animator.stopAllSounds("vacuumLoop")

    self.cooldownTimer = self.cooldownTime

    self.active = false
  end

  function vacuum:reset()
    animator.setAnimationState("vacuum", "idle")
    animator.stopAllSounds("vacuumLoop")
    activeItem.setItemForceRegions({})
    self.active = false
  end

  function vacuum:uninit()
    self:reset()
  end

  return vacuum
end
