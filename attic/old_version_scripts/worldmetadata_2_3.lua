function update(store)
  if store.worldTemplate.celestialParameters then
    -- Old initialization bugs can cause garbage planet / satellite numbers
    if store.worldTemplate.celestialParameters.coordinate.planet > 100 then
      store.worldTemplate.celestialParameters.coordinate.planet = 0
    end
    if store.worldTemplate.celestialParameters.coordinate.satellite > 100 then
      store.worldTemplate.celestialParameters.coordinate.satellite = 0
    end
  end

  return store
end
