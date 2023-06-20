function update(store)
  store.worldTemplate = store.planet
  store.worldTemplate.templateData = store.planet.config

  local coord = store.planet.config.skyParameters.coordinate
  if coord then
    -- If our world has a valid coordinate, completely rebuild the
    -- celestialParameters field from the database
    store.worldTemplate.celestialParameters = celestial.parameters({sector=coord.parentSystem.sector, location=coord.parentSystem.location, planet=coord.planetaryOrbitNumber, satellite=coord.satelliteOrbitNumber})

    -- Then, use this data to fill in missing fields in templateData (as it stores a world local copy anyway)
    
    local scanData = store.worldTemplate.celestialParameters.scanData

    store.worldTemplate.templateData.primaryBiomeName = scanData.primaryBiomeName
    store.worldTemplate.templateData.primaryBiomeHueShift = scanData.primaryBiomeHueShift
    store.worldTemplate.templateData.primarySurfaceLiquid = scanData.primarySurfaceLiquid
    store.worldTemplate.templateData.skyColoring = scanData.skyColoring
    store.worldTemplate.templateData.isBarren = scanData.isBarren
    store.worldTemplate.templateData.dayLength = scanData.dayLength

  else
    -- Otherwise, probably a ship world so it barely matters anyway

    store.worldTemplate.templateData.primaryBiomeName = ""
    store.worldTemplate.templateData.primaryBiomeHueShift = 0
    store.worldTemplate.templateData.primarySurfaceLiquid = 0
    store.worldTemplate.templateData.skyColoring = store.planet.config.skyParameters.skyColoring
    store.worldTemplate.templateData.isBarren = store.planet.config.skyParameters.skyType == "barren"
    store.worldTemplate.templateData.dayLength = store.planet.config.skyParameters.dayLength
  end


  return store
end
