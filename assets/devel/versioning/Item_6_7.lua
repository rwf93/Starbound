require "/scripts/versioningutils.lua"

-- This version: replace many types of generated weapons with their activeitem equivalents,
--               rename a few items, and convert .wav to .ogg for remaining generated guns/swords

generalItemConversions = {
  meatwine = "meatjuice",
  winebottle = "cordialbottle",
  wineglass = "cordialglass",
  whiskeybottle = "rootpopbottle",
  whiskeyflask = "rootpopflask",
  beerhead = "bevhead",
  ricewine = "ricevinegar",
  wartwine = "wartjuice",
  weyene = "oculemonade"
}

-- commented out entries are generated weapons to be converted next version
meleeWeaponConversions = {
  avianspear = "commonspear",
  aviansword = "commonshortsword",
  -- boneaxe = "boneaxe",
  -- bonehammer = "bonehammer",
  -- bonesaw = "bonesaw",
  -- bonesword = "bonesword",
  -- candycane = "candycane",
  commonaxe = "commonaxe",
  commonbroadsword = "commonbroadsword",
  commondagger = "commondagger",
  commonhammer = "commonhammer",
  commonshortsword = "commonshortsword",
  commonspear = "commonspear",
  -- crystalspear = "crystalspear",
  -- eyeaxe = "eyeaxe",
  -- eyesword = "eyesword",
  firesword = "rarebroadsword",
  floranmace = "commonhammer",
  floranprod = "uncommonspear",
  -- fryingpan = "fryingpan",
  glitchlargemace = "commonhammer",
  glitchsmallmace = "commonshortsword",
  mushroomsword = "uncommonshortsword",
  nightstick = "commonshortsword",
  npcflorantier1spear = "commonspear",
  npcmutantminerhammer = "commonhammer",
  rainbowspear = "uncommonspear",
  rustsword = "uncommonshortsword",
  rareaxe = "rareaxe",
  rarebroadsword = "rarebroadsword",
  raredagger = "raredagger",
  rarehammer = "rarehammer",
  rareshortsword = "rareshortsword",
  rarespear = "rarespear",
  shiv = "commondagger",
  -- slavesword = "slavesword",
  -- starcleaversword = "starcleaversword",
  -- steelchair = "steelchair",
  teslaspear = "rarespear",
  toxicbroadsword = "uncommonbroadsword",
  uncommonaxe = "uncommonaxe",
  uncommonbroadsword = "uncommonbroadsword",
  uncommondagger = "uncommondagger",
  uncommonhammer = "uncommonhammer",
  uncommonshortsword = "uncommonshortsword",
  uncommonspear = "uncommonspear",
  wallpainter = "commonbroadsword",
  watersword = "rarebroadsword"
}

-- commented out entries are generated weapons to be converted next version
rangedWeaponConversions = {
  avianblaster = "uncommonpistol",
  avianheavyblaster = "uncommonassaultrifle",
  -- boneassault = "boneassault",
  -- bonepistol = "bonepistol",
  -- boneshotgun = "boneshotgun",
  cellzapper = "uncommonsniperrifle",
  commonassaultrifle = "commonassaultrifle",
  commonburstrifle = "commonassaultrifle",
  commongrenadelauncher = "commongrenadelauncher",
  commonmachinepistol = "commonmachinepistol",
  commonpistol = "commonpistol",
  commonpulserifle = "commonassaultrifle",
  commonrocketlauncher = "commonrocketlauncher",
  commonshotgun = "commonshotgun",
  commonsniperrifle = "commonsniperrifle",
  -- crossbow = "crossbow",
  -- crossbowspecial = "crossbowspecial",
  -- crossbowwood = "crossbowwood",
  flamethrower = "flamethrower",
  florangrenadelauncher = "commongrenadelauncher",
  floranneedler = "commonassaultrifle",
  globelauncher = "commongrenadelauncher",
  lightningcoil = "commonassaultrifle",
  pulserifle = "commonassaultrifle",
  rareassaultrifle = "rareassaultrifle",
  raregrenadelauncher = "raregrenadelauncher",
  raremachinepistol = "raremachinepistol",
  rarepistol = "rarepistol",
  rarerocketlauncher = "rarerocketlauncher",
  rareshotgun = "rareshotgun",
  raresniperrifle = "raresniperrifle",
  revolver = "commonpistol",
  stingergun = "commonassaultrifle",
  shattergun = "commonshotgun",
  uncommonassaultrifle = "uncommonassaultrifle",
  uncommongrenadelauncher = "uncommongrenadelauncher",
  uncommonmachinepistol = "uncommonmachinepistol",
  uncommonpistol = "uncommonpistol",
  uncommonpulserifle = "uncommonassaultrifle",
  uncommonrocketlauncher = "uncommonrocketlauncher",
  uncommonshotgun = "uncommonshotgun",
  uncommonsniperrifle = "uncommonsniperrifle",
  uzi = "commonmachinepistol"
}

weaponTypeInferences = {
  ["lightning coil"] = "lightningcoil",
  ["pulse rifle"] = "pulserifle",
  ["revolver"] = "revolver"
}

function update(data)
  if generalItemConversions[data.name] then
    data.name = generalItemConversions[data.name]
  elseif data.name == "generatedsword" then
    local definitionName = extractDefinitionName(data.parameters)
    if definitionName then
      if meleeWeaponConversions[definitionName] then
        data.name = meleeWeaponConversions[definitionName]
        data.parameters = { level = data.parameters.level }
      end
    end
  elseif data.name == "generatedgun" then
    local definitionName = extractDefinitionName(data.parameters)
    if definitionName then
      if rangedWeaponConversions[definitionName] then
        data.name = rangedWeaponConversions[definitionName]
        data.parameters = { level = data.parameters.level }
      end
    end
  end

  if data.name == "generatedgun" or data.name == "generatedsword" then
    replacePatternInData(data.parameters, nil, ".wav", ".ogg")
    replacePatternInData(data.parameters, nil, "swing_onehanded", "swing_shortsword")
    replacePatternInData(data.parameters, nil, "swing_twohanded", "swing_broadsword")
  end

  return data
end

function extractDefinitionName(parameters)
  local definitionName = parameters.definition or parameters.definitionName
  if not definitionName and parameters.weaponType and weaponTypeInferences[string.lower(parameters.weaponType)] then
    definitionName = weaponTypeInferences[string.lower(parameters.weaponType)]
  end
  return definitionName
end
