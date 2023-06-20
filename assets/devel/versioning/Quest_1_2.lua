require "/scripts/versioningutils.lua"

function update(data)
  replacePatternInData(data, nil, ".gearup", "")

  return data
end
