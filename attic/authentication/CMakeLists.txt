INCLUDE_DIRECTORIES (
    ${STAR_CORE_INCLUDES}
    ${STAR_AUTHENTICATION_INCLUDES}
  )

SET (authentication_HEADERS
      StarAuthenticationDatabaseFacade.hpp
      StarAuthenticationKey.hpp
      StarAuthenticationService.hpp
      StarAuthenticationConnection.hpp
      StarClientAuthentication.hpp
      StarServerAuthentication.hpp
    )

SET (authentication_SOURCES
      StarAuthenticationKey.cpp
      StarAuthenticationService.cpp
      StarAuthenticationConnection.cpp
      StarClientAuthentication.cpp
      StarServerAuthentication.cpp
    )

ADD_LIBRARY (star_authentication ${authentication_SOURCES} ${authentication_HEADERS})
TARGET_LINK_LIBRARIES (star_authentication star)
