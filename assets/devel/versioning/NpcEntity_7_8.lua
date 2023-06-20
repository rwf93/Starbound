require "/scripts/versioningutils.lua"

function update(data)
  executeWhere(data, "basePercentage", nil, function(statModifier)
      statModifier.baseMultiplier = statModifier.basePercentage + 1.0
      statModifier.basePercentage = nil
    end)

  executeWhere(data, "effectivePercentage", nil, function(statModifier)
      statModifier.effectiveMultiplier = statModifier.effectivePercentage + 1.0
      statModifier.effectivePercentage = nil
    end)

  return data
end
