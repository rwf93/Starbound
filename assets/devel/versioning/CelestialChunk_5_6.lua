require "/scripts/versioningutils.lua"

function update(data)
  executeWhere(data, "layerBaseHeight", nil, function(entry)
      entry.dungeonXVariance = 500
    end)

  return data
end
