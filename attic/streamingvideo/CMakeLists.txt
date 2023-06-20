INCLUDE_DIRECTORIES (
    ${STAR_CORE_INCLUDES}
    ${STAR_STREAMINGVIDEO_INCLUDES}
    ${STAR_STREAMINGVIDEO_INCLUDES}/twitch
  )

SET (streamingvideo_HEADERS
        StarStreamingVideoController.hpp
        StarTwitchStreamingVideoController.hpp
    )

SET (streamingvideo_SOURCES
        StarStreamingVideoController.cpp
        StarTwitchStreamingVideoController.cpp
    )

ADD_LIBRARY (star_streamingvideo ${streamingvideo_SOURCES} ${streamingvideo_HEADERS})
TARGET_LINK_LIBRARIES (star_streamingvideo star)
