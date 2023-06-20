INCLUDE_DIRECTORIES (
    ${STAR_CORE_INCLUDES}
    ${STAR_STEAM_INCLUDES}
  )

SET (steam_HEADERS
        StarSteamCheck.hpp
    )

SET (steam_SOURCES
        StarSteamCheck.cpp
    )

ADD_LIBRARY (star_steam ${steam_SOURCES} ${steam_HEADERS})
TARGET_LINK_LIBRARIES (star_steam star)
