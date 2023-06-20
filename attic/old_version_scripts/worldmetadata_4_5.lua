function update(store)
  if store.worldTemplate.celestialParameters then
    if store.worldTemplate.celestialParameters.scanData then
      -- added weather pool information to world metadata
      -- if it's not there... just fake it
      if not store.worldTemplate.celestialParameters.scanData.weatherPool then
        store.worldTemplate.celestialParameters.scanData.weatherPool = vlist()
        -- close enough!
      end
    end
  end

  return store
end
