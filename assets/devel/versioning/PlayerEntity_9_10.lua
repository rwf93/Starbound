function update(data)
  local oldMetadata = versioning.loadVersionedJson("player/" .. data.uuid .. ".metadata")
  if oldMetadata then
    if not data.aiState then
      data.aiState = oldMetadata.ai
    end

    if not data.shipUpgrades then
      data.shipUpgrades = oldMetadata.shipUpgrades
    end
  end

  return data
end
