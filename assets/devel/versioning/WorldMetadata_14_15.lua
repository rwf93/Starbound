require "/scripts/versioningutils.lua"

function update(data)
  transformInData(data, "image", function(imagePath)
      local newPath = string.gsub(imagePath, "/celestial/system/terrestrial/dynamics/%d%d?", "/celestial/system/terrestrial/dynamics/temperate/" .. math.random(1,30))
      newPath = string.gsub(newPath, "/celestial/system/gas_giant/gas_giant_clouds.png", "/celestial/system/gas_giant/gas_giant_clouds_0.png")
      return newPath
    end)

  return data
end
