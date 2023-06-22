include_directories(
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
  )

set(star_base_HEADERS
    StarAnimatedPartSet.hpp
    StarAssets.hpp
    StarAssetSource.hpp
    StarBlocksAlongLine.hpp
    StarCellularLightArray.hpp
    StarCellularLighting.hpp
    StarCellularLiquid.hpp
    StarConfiguration.hpp
    StarDirectoryAssetSource.hpp
    StarMixer.hpp
    StarPackedAssetSource.hpp
    StarVersion.hpp
    StarVersionOptionParser.hpp
    StarWorldGeometry.hpp
  )

set(star_base_SOURCES
    StarAnimatedPartSet.cpp
    StarAssets.cpp
    StarCellularLighting.cpp
    StarConfiguration.cpp
    StarDirectoryAssetSource.cpp
    StarMixer.cpp
    StarPackedAssetSource.cpp
    StarVersionOptionParser.cpp
    StarWorldGeometry.cpp
  )

configure_file(StarVersion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
add_library(star_base OBJECT ${star_base_SOURCES} ${star_base_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
