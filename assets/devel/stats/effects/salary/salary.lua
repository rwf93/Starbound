require "/scripts/vec2.lua"

function init()
  self.item = config.getParameter("item")
  self.health = status.resource("health")
end

function update(dt)
  if self.health > status.resource("health") then
    world.spawnItem(self.item.name, mcontroller.position(), self.item.count)
  end
  self.health = status.resource("health")
end
