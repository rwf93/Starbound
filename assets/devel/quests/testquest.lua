require "/scripts/vec2.lua"

function init()
  quest.setObjectiveList({
    {"display a compass", true},
    {"display a progress bar", true},
    {"expand / contract quest log when clicked", true},
    {"stretch frame to fit", true},
    {"un-track quests from log", true},
    {"move money display", true},
    {"cool beans; it works", false}
  })
  -- quest.setObjectiveList({})
end

function update(dt)
  local toOrigin = world.distance({0, 900}, entity.position())
  quest.setCompassDirection(vec2.angle(toOrigin))
  -- quest.setCompassDirection()

  quest.setProgress(math.max(0, 1 - vec2.mag(toOrigin) / 1000))
  -- quest.setProgress()
end
