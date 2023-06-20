function update(store)
  store.celestialLog.unlockedSectors = {}
  for key, value in pairs(store.celestialLog.sectorUnlocks) do
    if (value) then
      table.insert(store.celestialLog.unlockedSectors, key)
    end
  end

  if store.celestialLog.currentWorld then
    store.celestialLog.currentWorld.sector = store.celestialLog.currentWorld.parentSystem.sector
    store.celestialLog.currentWorld.location = store.celestialLog.currentWorld.parentSystem.location
    store.celestialLog.currentWorld.planet = store.celestialLog.currentWorld.planetaryOrbitNumber
    store.celestialLog.currentWorld.satellite = store.celestialLog.currentWorld.satelliteOrbitNumber
  end

  if store.celestialLog.homeWorld then
    store.celestialLog.homeWorld.sector = store.celestialLog.homeWorld.parentSystem.sector
    store.celestialLog.homeWorld.location = store.celestialLog.homeWorld.parentSystem.location
    store.celestialLog.homeWorld.planet = store.celestialLog.homeWorld.planetaryOrbitNumber
    store.celestialLog.homeWorld.satellite = store.celestialLog.homeWorld.satelliteOrbitNumber
  end

  return store
end

