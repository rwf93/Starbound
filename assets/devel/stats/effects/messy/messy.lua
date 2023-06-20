require "/scripts/vec2.lua"
require "/scripts/pathutil.lua"

function init()
  self.object = config.getParameter("object")
  self.placementOffset = config.getParameter("offset", {0, 0})
  self.ready = false
  self.health = status.resource("health")
end

function floorPosition()
  local bounds = mcontroller.boundBox()
  local headPosition = findGroundPosition(mcontroller.position(), -5, 2, true)
  if not headPosition then
    return nil
  end
  return { headPosition[1], headPosition[2] + bounds[2] }
end

function update(dt)
  if self.health > status.resource("health") then
    self.ready = true
  end
  if self.ready then
    local position = floorPosition()
    if position then
      position = vec2.add(position, self.placementOffset)
      if world.placeObject(self.object, position) then
        self.ready = false
      end
    end
  end
  self.health = status.resource("health")
end
