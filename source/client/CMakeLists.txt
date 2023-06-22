include_directories(
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
    ${STAR_FRONTEND_INCLUDES}
  )

set(star_client_HEADERS
      StarClientApplication.hpp
    )

set(star_client_SOURCES
      StarClientApplication.cpp
    )

if(STAR_SYSTEM_WINDOWS)
  if(CMAKE_GENERATOR MATCHES "Visual Studio") # Ninja and possibly some others have problems building resources.
    set(star_client_RESOURCES
        starbound.rc
      )
  endif()
endif()

add_executable(starbound WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  $<TARGET_OBJECTS:star_application> $<TARGET_OBJECTS:star_rendering> $<TARGET_OBJECTS:star_windowing> $<TARGET_OBJECTS:star_frontend>
  ${star_client_HEADERS} ${star_client_SOURCES} ${star_client_RESOURCES})
target_link_libraries(starbound ${STAR_EXT_LIBS} ${STAR_EXT_GUI_LIBS})
