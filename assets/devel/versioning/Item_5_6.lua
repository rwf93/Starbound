require "/scripts/versioningutils.lua"

-- In this version: remove old shields, replace with activeitem shields

shieldDefinitionConversions = {
  ["eyeshield"] = "eyeshield",
  ["floranbasicshield"] = "floranshield",
  ["floranstrongshield"] = "floranshield",
  ["frostshield"] = "frostshield",
  ["glitchbasicshield"] = "commonsmallshield",
  ["glitchlordshield"] = "commonlargeshield",
  ["glitchstrongshield"] = "commonlargeshield",
  ["hylotlstrongshield"] = "commonlargeshield",
  ["mossshield"] = "commonsmallshield",
  ["mushroomshield"] = "mushroomshield",
  ["riotshield"] = "riotshield",
  ["seashellshield"] = "seashellshield",
  ["startershield"] = "startershield",
  ["tier1shield"] = "commonsmallshield",
  ["tier1woodshield"] = "commonsmallshield",
  ["tier2shield"] = "commonlargeshield",
  ["tier2woodshield"] = "commonlargeshield",
  ["tierxshield"] = "commonlargeshield"
}

shieldConversions = {
  ["glitchbasic"] = "commonsmallshield",
  ["glitchstrong"] = "commonlargeshield",
  ["glitchlord"] = "commonlargeshield",
  ["floranbasic"] = "floranshield",
  ["floranstrong"] = "floranshield",
  ["hylotlstrong"] = "commonlargeshield",
  ["riotshield"] = "riotshield",
  ["frostshield"] = "frostshield",
  ["moss"] = "commonsmallshield",
  ["mushroom"] = "mushroomshield",
  ["seashellshield"] = "seashellshield",
  ["eyepatch"] = "eyeshield",
  ["tieredshields/tier1"] = "commonsmallshield",
  ["tieredshields/tier1wood"] = "commonsmallshield",
  ["tieredshields/tier2"] = "commonlargeshield",
  ["tieredshields/tier2wood"] = "commonlargeshield",
  ["tieredshields/starter"] = "startershield",
  ["tieredshields/tierx"] = "commonlargeshield"
}

function extractShieldType(parameters)
  local imagePath = parameters.drawables[1].image
  local _, ia = imagePath:find("/randomgenerated/")
  local ib, _ = imagePath:find("/images/")
  if ia and ib then
    return imagePath:sub(ia + 1, ib - 1)
  else
    return "unknown"
  end
end

function update(data)
  if data.name == "generatedshield" then
    if data.parameters.definition then
      data.name = shieldDefinitionConversions[data.parameters.definition] or "commonsmallshield"
    else
      local typeName = extractShieldType(data.parameters)
      data.name = shieldConversions[typeName] or "commonsmallshield"
    end

    local level = data.parameters.level
    data.parameters = {level = level or 1}
    -- sb.logInfo("Found shield of type %s level %s, converting to %s", typeName, level, data.name)
  end

  return data
end
