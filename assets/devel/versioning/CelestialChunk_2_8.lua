require "/scripts/versioningutils.lua"

function update(data)
  -- 2 to 3
  replaceInData(data, "surfaceLiquid", 4, 0)
  replaceInData(data, "caveLiquid", 4, 0)
  replaceInData(data, "oceanLiquid", 4, 0)

  -- 3 to 5
  executeWhere(data, "oceanLiquid", nil, function(entry)
      entry.encloseLiquids = false
      entry.fillMicrodungeons = false
    end)
  replaceKeyInData(data, "caveLiquidCommonalityFactor", "caveLiquidSeedDensity")
  transformInData(data, "caveLiquidSeedDensity", function(density)
    if type(density) == "number" and density > 0 then
      return density / 4
    else
      return density
    end
  end)

  -- 5 to 6
  executeWhere(data, "layerBaseHeight", nil, function(entry)
      entry.dungeonXVariance = 500
    end)

  -- 6 to 7
  replaceInData(data, "dynamicsImages", "/celestial/system/terrestrial/dynamics/<num>.png", "/celestial/system/terrestrial/dynamics/temperate/<num>.png")
  transformInData(data, "dynamicsRange", function(dynamicsRange)
      if dynamicsRange[1] == 1 and dynamicsRange[2] == 40 then
        return {1, 30}
      else
        return dynamicsRange
      end
    end)

  -- 7 to 8
  replaceInData(data, nil, "scifidungeon", "apexresearchlab")

  return data
end
