find_path(AVFORMAT_INCLUDE_DIR
    libavformat/avformat.h
    PATHS
        /usr/include
        /usr/local/include
        /usr/include/x86_64-linux-gnu
)

find_library(AVFORMAT_LIBRARY
    avformat
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

# set(FFMPEG_INCLUDE_DIRS ${AVFORMAT_INCLUDE_DIR}  CACHE INTERNAL "FFmpeg include dirs")
# set(FFMPEG_LIBRARIES ${AVFORMAT_LIBRARY} CACHE INTERNAL "FFmpeg libraries")

# Find avcodec
find_path(AVCODEC_INCLUDE_DIR
    libavcodec/avcodec.h
    PATHS
        /usr/include
        /usr/local/include
        /usr/include/x86_64-linux-gnu
)

find_library(AVCODEC_LIBRARY
    avcodec
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

# Find avutil
find_path(AVUTIL_INCLUDE_DIR
    libavutil/avutil.h
    PATHS
        /usr/include
        /usr/local/include
        /usr/include/x86_64-linux-gnu
)

find_library(AVUTIL_LIBRARY
    avutil
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

# Find avutil
find_path(SWRESAMPLE_INCLUDE_DIR
    libswresample/swresample.h
    PATHS
        /usr/include
        /usr/local/include
        /usr/include/x86_64-linux-gnu
)

find_library(SWRESAMPLE_LIBRARY
    swresample
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
)

# Set up final include and lib variables
set(FFMPEG_INCLUDE_DIRS
    ${AVFORMAT_INCLUDE_DIR}
    ${AVCODEC_INCLUDE_DIR}
    ${AVUTIL_INCLUDE_DIR}
    ${SWRESAMPLE_INCLUDE_DIR}
    CACHE INTERNAL "FFmpeg include dirs"
)

set(FFMPEG_LIBRARIES
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
    CACHE INTERNAL "FFmpeg libraries"
)
