require "/scripts/versioningutils.lua"

function update(data)
  replaceInData(data, nil, "scifidungeon", "apexresearchlab")

  return data
end
