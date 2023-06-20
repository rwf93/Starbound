require "/scripts/versioningutils.lua"

function update(data)
  local defaultSettings = root.assetJson("/player.config:statusControllerSettings")
  transformInData(data, "statusController", function(statusStorage)
      removeFromData(statusStorage, "shieldHealth")
      statusStorage.statusProperties.slows = nil
      for k,v in pairs(defaultSettings.statusProperties) do
        statusStorage.statusProperties[k] = v
      end
      return statusStorage
    end)

  return data
end
