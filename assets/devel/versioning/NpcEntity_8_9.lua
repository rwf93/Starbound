function update(data)
  for key, value in pairs(data.dropPools) do
    if not root.isTreasurePool(value) then
      data.dropPools[key] = "villagertreasure"
    end
  end

  return data
end
