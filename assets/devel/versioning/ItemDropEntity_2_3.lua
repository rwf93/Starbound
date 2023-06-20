function update(data)
  if data.timeToLive > 0 then
    data.mode = "available"
  else
    data.mode = "dead"
  end

  data.dropAge = jobject()
  data.dropAge.lastEpochTime = nil
  data.dropAge.elapsedTime = 0.0

  data.ageItemsTimer = jobject()
  data.ageItemsTimer.lastEpochTime = nil
  data.ageItemsTimer.elapsedTime = 0.0

  return data
end

