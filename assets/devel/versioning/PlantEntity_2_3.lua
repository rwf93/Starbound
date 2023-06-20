require "/scripts/versioningutils.lua"

function update(data)
  replacePatternInData(data, nil, ".wav", ".ogg")

  return data
end
