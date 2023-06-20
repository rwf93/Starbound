function update(store)
  if store.worldTemplate.celestialParameters then
    if store.worldTemplate.celestialParameters.scanData then
      -- added detached Biome information to world metadata
      -- if it's not there... just fake it
      if not store.worldTemplate.celestialParameters.scanData.detachedBiome then
        store.worldTemplate.celestialParameters.scanData.detachedBiome = ""
        -- close enough!
      end
    end
  end

  return store
end
