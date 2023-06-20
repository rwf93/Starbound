require "/scripts/versioningutils.lua"

standardMelee = {
  ["dagger"] = "dagger",
  ["shortsword"] = "shortsword",
  ["broadsword"] = "broadsword",
  ["spear"] = "spear",
  ["axe"] = "axe",
  ["hammer"] = "hammer",
  ["testbroadsword"] = "broadsword"
}

specialMelee = {
  ["avian sword"] = "aviansword",
  ["avian spear"] = "avianspear",
  ["bonesaw"] = "bonesaw",
  ["bone sword"] = "bonesword",
  ["candy cane"] = "candycane",
  ["crystal spear"] = "crystalspear",
  ["eye axe"] = "eyeaxe",
  ["floran prod"] = "floranprod",
  ["frying pan"] = "fryingpan",
  ["glitch heavy mace"] = "glitchlargemace",
  ["glitch mace"] = "glitchsmallmace",
  ["nightstick"] = "nightstick",
  ["floran spear"] = "npcflorantier1spear",
  ["rainbow spear"] = "rainbowspear",
  ["rust sword"] = "rustsword",
  ["shiv"] = "shiv",
  ["steel chair"] = "steelchair",
  ["toxic broadsword"] = "toxicbroadsword",
  ["fire sword"] = "firesword",
  ["mushroom sword"] = "mushroomsword",
  ["water sword"] = "watersword",
  ["eye sword"] = "eyesword",
  ["floran mace"] = "floranmace",
  ["heroic sword"] = "slavesword",
  ["star cleaver"] = "starcleaversword",
  ["tesla spear"] = "teslaspear",
  ["bone axe"] = "boneaxe",
  ["bone hammer"] = "bonehammer"
}

standardRanged = {
  ["assault rifle"] = "assaultrifle",
  ["plasma rifle"] = "assaultrifle",
  ["burst rifle"] = "assaultrifle",
  ["grenade launcher"] = "grenadelauncher",
  ["machine pistol"] = "machinepistol",
  ["plasma m. pistol"] = "machinepistol",
  ["pistol"] = "pistol",
  ["plasma pistol"] = "pistol",
  ["rocket launcher"] = "rocketlauncher",
  ["shotgun"] = "shotgun",
  ["plasma shotgun"] = "shotgun",
  ["sniper rifle"] = "sniperrifle",
  ["plasma s.rifle"] = "sniperrifle"
}

specialRanged = {
  ["avian blaster"] = "avianblaster",
  ["avian heavy blaster"] = "avianheavyblaster",
  ["bone rifle"] = "boneassault",
  ["bone pistol"] = "bonepistol",
  ["bone shotgun"] = "boneshotgun",
  ["cellzapper"] = "cellzapper",
  ["flamethrower"] = "flamethrower",
  ["floran grenade launcher"] = "florangrenadelauncher",
  ["needler"] = "floranneedler",
  ["globe launcher"] = "globelauncher",
  ["shattergun"] = "shattergun",
  ["stinger gun"] = "stingergun",
  ["uzi"] = "uzi"
}

function update(data)
  if data.name == "generatedsword" then
    local weaponType = string.lower(data.parameters.weaponType)
    local generatedType
    if standardMelee[weaponType] then
      generatedType = string.format("%s%s", string.lower(data.parameters.rarity), standardMelee[weaponType])
    elseif specialMelee[weaponType] then
      generatedType = specialMelee[weaponType]
    else
      return data
    end

    local newData = root.createItem({"generatedsword", 1, {definition=generatedType}}, data.parameters.level)
    newData.parameters.shortdescription = data.parameters.shortdescription

    return newData
  elseif data.name == "generatedgun" then
    local weaponType = string.lower(data.parameters.weaponType)
    local generatedType
    if weaponType == "crossbow" then
      if data.parameters.rarity == "uncommon" then
        generatedType = "crossbowspecial"
      else
        generatedType = "crossbow"
      end
    elseif standardRanged[weaponType] then
      local rarity = string.lower(data.parameters.rarity)
      if rarity == "legendary" then rarity = "rare" end
      generatedType = string.format("%s%s", rarity, standardRanged[weaponType])
    elseif specialRanged[weaponType] then
      generatedType = specialRanged[weaponType]
    else
      return data
    end

    local newData = root.createItem({"generatedgun", 1, {definition=generatedType}}, data.parameters.level)
    newData.parameters.shortdescription = data.parameters.shortdescription

    return newData
  end

  return data
end
