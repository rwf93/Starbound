require "/scripts/versioningutils.lua"

function update(data)
  replaceKeyInData(data, "radius", "sectorRadius")
  transformInData(data, "wormSizeRange", function(oldsize) return { oldsize[1] * 2, oldsize[2] * 2 } end)

  return data
end
