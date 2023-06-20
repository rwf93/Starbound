require "/scripts/versioningutils.lua"

function update(data)
  transformInData(data, "biomes", function(biomeList)
      local seed = math.random(1, 4000000000) -- just use any old seed, doesn't really matter
      for _, biome in pairs(biomeList) do
        seed = seed + 13 -- it's more magical when you iterate by magical numbers
        if biome.baseName then
          local newBiome = root.createBiome(biome.baseName, seed, 0, 1)
          if newBiome then
            biome.surfacePlaceables = newBiome.surfacePlaceables
            biome.undergroundPlaceables = newBiome.undergroundPlaceables
            biome.spawnProfile = newBiome.spawnProfile
          end
        end
      end
      return biomeList
    end)

  return data
end
