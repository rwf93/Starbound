include_directories(
    ${STAR_CORE_INCLUDES}
    ${STAR_STEAM_INCLUDES}
  )

set(steam_HEADERS
        StarSteamCheck.hpp
    )

set(steam_SOURCES
        StarSteamCheck.cpp
    )

add_library(star_steam ${steam_SOURCES} ${steam_HEADERS})
target_link_libraries(star_steam star)
