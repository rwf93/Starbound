require "/scripts/versioningutils.lua"

function update(data)
  if hasPath(data, {"npcVariant", "level"}) and hasPath(data, {"statusController", "persistentEffectCategories", "innate"}) then
    local level = data.npcVariant.level
    data.statusController.persistentEffectCategories.innate = {
      {
        stat = "powerMultiplier",
        amount = root.evalFunction("npcLevelPowerMultiplierModifier", level)
      },
      {
        stat = "protection",
        baseMultiplier = root.evalFunction("npcLevelProtectionMultiplier", level)
      },
      {
        stat = "maxHealth",
        baseMultiplier = root.evalFunction("npcLevelHealthMultiplier", level)
      },
      {
        stat = "maxEnergy",
        baseMultiplier = root.evalFunction("npcLevelEnergyMultiplier", level)
      }
    }
  end

  return data
end
