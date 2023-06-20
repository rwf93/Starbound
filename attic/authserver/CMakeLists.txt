INCLUDE_DIRECTORIES (
    ${STAR_CORE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_AUTHENTICATION_INCLUDES}
  )

SET (authentication_server_HEADERS
      StarAuthenticationDatabase.hpp
      StarAuthenticationServer.hpp
      StarDatabaseConnector.hpp
    )

SET (authentication_server_SOURCES
      StarAuthenticationDatabase.cpp
      StarAuthenticationServer.cpp
      StarDatabaseConnector.cpp
    )

ADD_EXECUTABLE (starbound_auth_keygen keygen.cpp ${authentication_server_SOURCES} ${authentication_server_HEADERS})
ADD_EXECUTABLE (starbound_auth main.cpp ${authentication_server_SOURCES} ${authentication_server_HEADERS})

TARGET_LINK_LIBRARIES (starbound_auth_keygen star_authentication ${PQ_LIBRARY})
TARGET_LINK_LIBRARIES (starbound_auth star_authentication ${PQ_LIBRARY})
