function youwin()
  self.wintime = 5
end

function update(dt)
  if self.wintime and self.wintime > 0 then
    self.wintime = self.wintime - dt
    world.spawnItem("money", vec2.add(object.position(), {0, 3}), 5)
  end
end
