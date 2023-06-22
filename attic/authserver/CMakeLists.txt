include_directories(
    ${STAR_CORE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_AUTHENTICATION_INCLUDES}
  )

set(authentication_server_HEADERS
      StarAuthenticationDatabase.hpp
      StarAuthenticationServer.hpp
      StarDatabaseConnector.hpp
    )

set(authentication_server_SOURCES
      StarAuthenticationDatabase.cpp
      StarAuthenticationServer.cpp
      StarDatabaseConnector.cpp
    )

add_executable(starbound_auth_keygen keygen.cpp ${authentication_server_SOURCES} ${authentication_server_HEADERS})
add_executable(starbound_auth main.cpp ${authentication_server_SOURCES} ${authentication_server_HEADERS})

target_link_libraries(starbound_auth_keygen star_authentication ${PQ_LIBRARY})
target_link_libraries(starbound_auth star_authentication ${PQ_LIBRARY})
