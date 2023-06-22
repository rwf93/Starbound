include_directories(
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )

add_executable(asset_packer
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_packer.cpp)
target_link_libraries(asset_packer ${STAR_EXT_LIBS})

add_executable(asset_unpacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_unpacker.cpp)
target_link_libraries(asset_unpacker ${STAR_EXT_LIBS})

add_executable(dump_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  dump_versioned_json.cpp)
target_link_libraries(dump_versioned_json ${STAR_EXT_LIBS})

add_executable(game_repl
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  game_repl.cpp)
target_link_libraries(game_repl ${STAR_EXT_LIBS})

add_executable(make_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  make_versioned_json.cpp)
target_link_libraries(make_versioned_json ${STAR_EXT_LIBS})

add_executable(planet_mapgen
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  planet_mapgen.cpp)
target_link_libraries(planet_mapgen ${STAR_EXT_LIBS})

add_executable(render_terrain_selector
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  render_terrain_selector.cpp)
target_link_libraries(render_terrain_selector ${STAR_EXT_LIBS})

add_executable(update_tilesets
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  update_tilesets.cpp tileset_updater.cpp)
target_link_libraries(update_tilesets ${STAR_EXT_LIBS})

add_executable(fix_embedded_tilesets
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  fix_embedded_tilesets.cpp)
target_link_libraries(fix_embedded_tilesets ${STAR_EXT_LIBS})

add_executable(world_benchmark
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  world_benchmark.cpp)
target_link_libraries(world_benchmark ${STAR_EXT_LIBS})

add_executable(generation_benchmark
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  generation_benchmark.cpp)
target_link_libraries(generation_benchmark ${STAR_EXT_LIBS})

add_executable(dungeon_generation_benchmark
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  dungeon_generation_benchmark.cpp)
target_link_libraries(dungeon_generation_benchmark ${STAR_EXT_LIBS})

add_executable(map_grep
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  map_grep.cpp)
target_link_libraries(map_grep ${STAR_EXT_LIBS})

add_executable(word_count
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  word_count.cpp)
target_link_libraries(word_count ${STAR_EXT_LIBS})
