require "/scripts/vec2.lua"

function init()
  
end

function update()
  
end

function control(direction)
  mcontroller.approachVelocity(vec2.mul(vec2.norm(direction), config.getParameter("maxSpeed")), config.getParameter("controlForce"))
end

function controlTo(position)
  local offset = world.distance(position, mcontroller.position())
  mcontroller.approachVelocity(vec2.mul(vec2.norm(offset), config.getParameter("maxSpeed")), config.getParameter("controlForce"))
end

function isDrone()
  return true
end
