include_directories(
    ${STAR_CORE_INCLUDES}
    ${STAR_AUTHENTICATION_INCLUDES}
  )

set(authentication_HEADERS
      StarAuthenticationDatabaseFacade.hpp
      StarAuthenticationKey.hpp
      StarAuthenticationService.hpp
      StarAuthenticationConnection.hpp
      StarClientAuthentication.hpp
      StarServerAuthentication.hpp
    )

set(authentication_SOURCES
      StarAuthenticationKey.cpp
      StarAuthenticationService.cpp
      StarAuthenticationConnection.cpp
      StarClientAuthentication.cpp
      StarServerAuthentication.cpp
    )

add_library(star_authentication ${authentication_SOURCES} ${authentication_HEADERS})
target_link_libraries(star_authentication star)
