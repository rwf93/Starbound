require "/scripts/versioningutils.lua"

function update(data)
  replacePatternInData(data, nil, "UniqueWorld", "InstanceWorld")
  replacePatternInData(data, nil, "MissionWorld", "InstanceWorld")

  return data
end
