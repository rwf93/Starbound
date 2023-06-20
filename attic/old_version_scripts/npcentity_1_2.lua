function update(store)
  local npcConfig = root.npcConfig(store.npcVariant.typeName)

  -- 20140126 Didn't store leveled baseMaxHealth and baseMaxEnergy
  store.npcVariant.baseMaxEnergy = npcConfig.statusParameters.baseMaxEnergy * root.evalFunction("npcLevelHealthMultiplier", store.npcVariant.level)
  store.npcVariant.baseMaxHealth = npcConfig.statusParameters.baseMaxHealth * root.evalFunction("npcLevelEnergyMultiplier", store.npcVariant.level)
  store.status.healthSchema.max = store.npcVariant.baseMaxHealth
  store.status.energySchema.max = store.npcVariant.baseMaxEnergy

  return store
end
