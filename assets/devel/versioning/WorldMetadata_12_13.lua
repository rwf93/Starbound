require "/scripts/versioningutils.lua"

function update(data)
  executeWhere(data, nil, "microDungeon", function(entry)
      if type(entry[2]) == "string" then
        entry[2] = {entry[2]}
      end
    end)

  return data
end
