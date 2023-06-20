require "/scripts/versioningutils.lua"

function update(data)
  -- If the object looks like a container but does not have ageItemsTimer
  if data.items and data.craftingProgress and not data.ageItemsTimer then
    -- Then add it
    data.ageItemsTimer = jobject()
    data.ageItemsTimer.lastEpochTime = nil
    data.ageItemsTimer.elapsedTime = 0.0
  end
  return data
end


