require "/scripts/versioningutils.lua"

function update(data)
  data.inboundWireNodes = nil
  data.outboundWireNodes = nil

  data.inputWireNodes = jarray()
  data.outputWireNodes = jarray()

  return data
end


