require "/scripts/util.lua"

function die(smash)
  local pos = entity.position()
  local spreadRange = config.getParameter("spreadRange", 2)
  local candidateSpaces = {}
  for x = pos[1] - spreadRange, pos[1] + spreadRange do
    for y = pos[2] - spreadRange, pos[2] + spreadRange do
      table.insert(candidateSpaces, {x, y})
    end
  end
  
  shuffle(candidateSpaces)

  local toPlace = config.getParameter("spreadFactor", 2)
  while #candidateSpaces > 0 and toPlace > 0 do
    if world.placeObject("greycapsule", candidateSpaces[1], 1, {}) then
      toPlace = toPlace - 1
    end
    table.remove(candidateSpaces, 1)
  end
end
