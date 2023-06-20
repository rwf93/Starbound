function update(data)
  local config = root.npcConfig(data.npcVariant.typeName)
  local scriptConfig = config.scriptConfig or {}
  data.aggressive = scriptConfig.aggressive or false

  return data
end
