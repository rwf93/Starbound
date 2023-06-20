require "/scripts/versioningutils.lua"

function update(data)
  local itemList = data.npcVariant.items

  local newItems = {}
  for k, v in pairs(data.npcVariant.items) do
    newItems[k] = {
        __id = "Item",
        __version = 5,
        __content = v
      }
  end
  data.npcVariant.items = newItems

  return data
end
