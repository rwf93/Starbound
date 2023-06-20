function update(data)
  data.shipUpgrades.crewSize = 2 * math.max(data.shipUpgrades.shipLevel - 2, 1)

  return data
end
