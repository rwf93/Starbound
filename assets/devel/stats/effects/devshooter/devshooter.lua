require "/scripts/vec2.lua"

function init()
  script.setUpdateDelta(20)
end

function update(dt)
  local targetIds = world.entityQuery(mcontroller.position(), 30, {
    withoutEntityId = entity.id(),
    includedTypes = {"creature"},
    order = "nearest"
  })

  for i,id in ipairs(targetIds) do
    if entity.isValidTarget(id) and not world.lineTileCollision(mcontroller.position(), world.entityPosition(id)) then
      local directionTo = vec2.norm(world.distance(world.entityPosition(id), mcontroller.position()))
      world.spawnProjectile("teslabolt", mcontroller.position(), entity.id(), directionTo, false, {speed = 100, power = 999})
    end
  end
end

function uninit()
  
end
