require "/scripts/versioningutils.lua"

function update(data)
  -- 8 to 9
  replaceInData(data, "surfaceLiquid", 4, 0)
  replaceInData(data, "caveLiquid", 4, 0)
  replaceInData(data, "oceanLiquid", 4, 0)

  -- 9 to 10
  replaceKeyInData(data, "radius", "sectorRadius")
  transformInData(data, "wormSizeRange", function(oldsize) return { oldsize[1] * 2, oldsize[2] * 2 } end)

  -- 10 to 12
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

  -- 12 to 13
  executeWhere(data, nil, "microDungeon", function(entry)
      if type(entry[2]) == "string" then
        entry[2] = {entry[2]}
      end
    end)

  -- 13 to 14
  executeWhere(data, "layerBaseHeight", nil, function(entry)
      entry.dungeonXVariance = 500
    end)

  -- 14 to 15 (sky parameters stuff)
  transformInData(data, "image", function(imagePath)
      local newPath = string.gsub(imagePath, "/celestial/system/terrestrial/dynamics/%d%d?", "/celestial/system/terrestrial/dynamics/temperate/" .. math.random(1,30))
      newPath = string.gsub(newPath, "/celestial/system/gas_giant/gas_giant_clouds.png", "/celestial/system/gas_giant/gas_giant_clouds_0.png")
      return newPath
    end)

  if data.worldTemplate.skyParameters then
    transformInData(data.worldTemplate.skyParameters, "scale", function(scale)
        if type(scale) == "table" then
          return scale[1]
        end
      end)
    replaceKeyInData(data.worldTemplate.skyParameters, "drawables", "layers")
  end

  -- 15 to 16
  replaceInData(data, nil, "scifidungeon", "apexresearchlab")

  return data
end
