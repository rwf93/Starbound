require "/scripts/versioningutils.lua"

function update(data)
  local stats = data.stats or {}
  data.stats = stats

  local prefix = "tenants.type."
  local badKeys = {}

  for key, stat in pairs(stats) do
    if key:sub(1, prefix:len()) == prefix then
      table.insert(badKeys, key)
    end
  end

  for _,key in pairs(badKeys) do
    jremove(stats, key)
  end

  return data
end
