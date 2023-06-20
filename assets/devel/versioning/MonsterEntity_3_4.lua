function update(data)
  if type(data.dropPool) == "string" then
    if not root.isTreasurePool(data.dropPool) then
      data.dropPool = "basicMonsterTreasure"
    end
  elseif data.dropPool then
    for key, value in pairs(data.dropPool) do
      if not root.isTreasurePool(value) then
        data.dropPool[key] = "basicMonsterTreasure"
      end
    end
  end

  return data
end
