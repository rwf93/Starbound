require "/scripts/versioningutils.lua"

function update(data)
  executeWhere(data, "oceanLiquid", nil, function(data) data.encloseOcean = false end)

  return data
end
