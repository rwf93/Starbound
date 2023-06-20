function update(data)
  if not data.worldTemplate or not data.worldTemplate.regionData or not data.worldTemplate.regionData.biomes then
    return data
  end

  local biomes = data.worldTemplate.regionData.biomes
  for _, biome in ipairs(biomes) do
    if biome.surfacePlaceables then
      biome.surfacePlaceables.ceilingGrassMod = biome.surfacePlaceables.ceilingGrassMod or 0
      biome.surfacePlaceables.ceilingGrassModDensity = biome.surfacePlaceables.ceilingGrassModDensity or 0
    end
    if biome.undergroundPlaceables then
      biome.undergroundPlaceables.ceilingGrassMod = biome.undergroundPlaceables.ceilingGrassMod or 0
      biome.undergroundPlaceables.ceilingGrassModDensity = biome.undergroundPlaceables.ceilingGrassModDensity or 0
    end
  end
  
  return data
end
