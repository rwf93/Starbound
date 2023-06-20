require "/scripts/versioningutils.lua"

function update(data)
  data.animatorState.transformationGroupData = jobject()
  return data
end

