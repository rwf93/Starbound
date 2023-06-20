require "/scripts/vec2.lua"

function init()
  self.inputPosition = config.getParameter("inputPosition")
  self.outputPosition = config.getParameter("outputPosition")

  if object.direction() < 0 then
    self.inputPosition, self.outputPosition = self.outputPosition, self.inputPosition
  end

  local pos = entity.position()
  self.inputPosition = vec2.add(vec2.add(self.inputPosition, pos), {0.5, 0.5})
  self.outputPosition = vec2.add(vec2.add(self.outputPosition, pos), {0.5, 0.5})

  self.pumpTimer = 0
  self.pumpFrequency = config.getParameter("pumpFrequency", 0.25)

  storage.pumping = storage.pumping or false
end

function update(dt)
  -- world.debugPoint(self.inputPosition, "blue")
  -- world.debugPoint(self.outputPosition, "green")

  if object.isInputNodeConnected(0) then
    object.setInteractive(false)
    storage.pumping = object.getInputNodeLevel(0)
  else
    object.setInteractive(true)
  end

  local pumpAnimationState = storage.pumping and ((world.liquidAt(self.inputPosition) and not world.pointCollision(self.outputPosition)) and "pump" or "error") or "idle"
  animator.setAnimationState("pumping", pumpAnimationState)

  self.pumpTimer = math.max(0, self.pumpTimer - dt)
  if storage.pumping and self.pumpTimer == 0 then
    pump()
    self.pumpTimer = self.pumpFrequency
  end
end

function onInteraction(args)
  storage.pumping = not storage.pumping
end

function uninit()

end

function pump()
  local liquid = world.liquidAt(self.inputPosition)
  if liquid and not world.pointCollision(self.outputPosition) then
    world.destroyLiquid(self.inputPosition)
    world.spawnLiquid(self.outputPosition, table.unpack(liquid))
  end
end
