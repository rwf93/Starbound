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

  -- extra versioning for previous sky parameters change

  if data.worldTemplate.skyParameters then
    transformInData(data.worldTemplate.skyParameters, "scale", function(scale)
        if type(scale) == "table" then
          return scale[1]
        end
      end)
    replaceKeyInData(data.worldTemplate.skyParameters, "drawables", "layers")
  end

  return data
end
