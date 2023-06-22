include_directories(
    ${STAR_CORE_INCLUDES}
    ${STAR_STREAMINGVIDEO_INCLUDES}
    ${STAR_STREAMINGVIDEO_INCLUDES}/twitch
  )

set(streamingvideo_HEADERS
        StarStreamingVideoController.hpp
        StarTwitchStreamingVideoController.hpp
    )

set(streamingvideo_SOURCES
        StarStreamingVideoController.cpp
        StarTwitchStreamingVideoController.cpp
    )

add_library(star_streamingvideo ${streamingvideo_SOURCES} ${streamingvideo_HEADERS})
target_link_libraries(star_streamingvideo star)
