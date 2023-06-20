require "/scripts/versioningutils.lua"

function update(data)
  executeWhere(data, "playerStartSearchRegion", nil, function(innerData)
      local regions = jarray()
      table.insert(regions, innerData.playerStartSearchRegion)
      innerData.playerStartSearchRegions = regions
      innerData.playerStartSearchRegion = nil
    end)

  return data
end
