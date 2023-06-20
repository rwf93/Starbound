function init()
  local materialSpaces = {}
  for x = -1, 1 do
    for y = -1, 1 do
      if x ~= 0 or y ~= 0 then
        table.insert(materialSpaces, {{x, y}, "metamaterial:objectsolid"})
      end
    end
  end
  object.setMaterialSpaces(materialSpaces)
end

function update(dt)
  object.setMaterialSpaces({})
  object.smash()
end
