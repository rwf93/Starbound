function init()

end

function update(dt)
  if not storage.cratePlaced then
    local pos = entity.position()
    local placepos = {pos[1], pos[2] + 2}
    local success = world.placeObject("stackercrate", placepos, 1, {})
    if success then
      storage.cratePlaced = true
    end
  end
end
