function update(data)
  local newLog = {
    playTime = data.playTime,
    deathCount = data.log.statistic.deathCount or 0,
    scannedObjects = data.scannedObjects,
    radioMessages = jarray()
  }
  data.log = newLog
  data.playTime = nil
  data.scannedObjects = nil

  return data
end
