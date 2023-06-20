require "/scripts/vec2.lua"

function init()
  updateDrone()
end

function uninit()
  -- sb.logInfo("Uninitializing metagun")
end

function update(dt, fireMode, shiftHeld)
  -- sb.logInfo("Updating metagun with fireMode %s and shiftHeld %s", fireMode, shiftHeld)

  updateDrone()
  updateAim()
  updateControl(fireMode)
end

function activate(fireMode, shiftHeld)
  if storage.droneId == nil then
    launchDrone()
  else
    self.controlPosition = world.distance(activeItem.ownerAimPosition(), mcontroller.position())
    self.droneControlPosition = world.entityPosition(storage.droneId)
  end
end

function launchDrone()
  local launchAngle = vec2.rotate({1, 0}, self.aimAngle)
  launchAngle[1] = launchAngle[1] * self.aimDirection
  storage.droneId = world.spawnProjectile(
    "drone",
    mcontroller.position(),
    activeItem.ownerEntityId(),
    launchAngle,
    false,
    {}
  )

end

function updateDrone()
  if storage.droneId and not (world.entityExists(storage.droneId) and world.callScriptedEntity(storage.droneId, "isDrone")) then
    storage.droneId = nil
  end

  activeItem.setCameraFocusEntity(storage.droneId)

  if storage.droneId then
    animator.setAnimationState("heldItem", "controller")
  else
    animator.setAnimationState("heldItem", "drone")
    activeItem.setCursor(nil)
  end
end

function updateAim()
  self.aimAngle, self.aimDirection = activeItem.aimAngleAndDirection(0, activeItem.ownerAimPosition())
  if storage.droneId then
    activeItem.setArmAngle(0)
  else
    activeItem.setArmAngle(self.aimAngle)
  end
  activeItem.setFacingDirection(self.aimDirection)
end

function updateControl(fireMode)
  if fireMode ~= "primary" then
    self.controlPosition = nil
  end

  if storage.droneId and self.controlPosition then
    local newControlPosition = world.distance(activeItem.ownerAimPosition(), mcontroller.position())
    world.callScriptedEntity(storage.droneId, "controlTo", vec2.add(self.droneControlPosition, vec2.sub(newControlPosition, self.controlPosition)))
  end

  -- if storage.droneId and self.controlPosition then
  --   local newControlPosition = world.distance(activeItem.ownerAimPosition(), mcontroller.position())
  --   world.callScriptedEntity(storage.droneId, "control", vec2.sub(newControlPosition, self.controlPosition))
  --   self.controlPosition = newControlPosition
  -- end

  -- if storage.droneId and self.controlPosition then
  --   local newControlPosition = world.distance(activeItem.ownerAimPosition(), mcontroller.position())
  --   local controlOffset = vec2.sub(newControlPosition, self.controlPosition)
  --   if vec2.mag(controlOffset) > 1 then
  --     if math.abs(controlOffset[1]) > math.abs(controlOffset[2]) then
  --       if controlOffset[1] > 0 then
  --         world.callScriptedEntity(storage.droneId, "control", {1, 0})
  --         activeItem.setCursor("/cursors/joystickright.cursor")
  --       else
  --         world.callScriptedEntity(storage.droneId, "control", {-1, 0})
  --         activeItem.setCursor("/cursors/joystickleft.cursor")
  --       end
  --     else
  --       if controlOffset[2] > 0 then
  --         world.callScriptedEntity(storage.droneId, "control", {0, 1})
  --         activeItem.setCursor("/cursors/joystickup.cursor")
  --       else
  --         world.callScriptedEntity(storage.droneId, "control", {0, -1})
  --         activeItem.setCursor("/cursors/joystickdown.cursor")
  --       end
  --     end
  --   else
  --     activeItem.setCursor("/cursors/joystick.cursor")
  --   end
  -- elseif storage.droneId then
  --   activeItem.setCursor("/cursors/joystick.cursor")
  -- end
end
