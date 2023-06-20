require "/scripts/versioningutils.lua"

function update(data)
  data.wireLevels = nil
  data.wireState = nil

  data.inboundWireNodes = jarray()
  data.outboundWireNodes = jarray()

  return data
end


