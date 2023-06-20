require "/scripts/versioningutils.lua"

function update(data)
  replaceKeyInData(data, "caveLiquidCommonalityFactor", "caveLiquidSeedDensity")
    transformInData(data, "caveLiquidSeedDensity", function(density)
      if type(density) == "number" and density > 0 then
        return density / 4
      else
        return density
      end
    end)
  replaceKeyInData(data, "encloseOcean", "encloseLiquids")
  executeWhere(data, "encloseLiquids", nil, function(entry)
      entry.fillMicrodungeons = false
    end)

  return data
end
