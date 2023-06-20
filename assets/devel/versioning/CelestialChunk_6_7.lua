require "/scripts/versioningutils.lua"

function update(data)
  replaceInData(data, "dynamicsImages", "/celestial/system/terrestrial/dynamics/<num>.png", "/celestial/system/terrestrial/dynamics/temperate/<num>.png")
  transformInData(data, "dynamicsRange", function(dynamicsRange)
      if dynamicsRange[1] == 1 and dynamicsRange[2] == 40 then
        return {1, 30}
      else
        return dynamicsRange
      end
    end)

  return data
end
