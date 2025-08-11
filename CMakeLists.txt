cmake_minimum_required(VERSION 3.13)
project(AJ_Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the output directory for executables to "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

#include all header files in include dir
include_directories(${PROJECT_SOURCE_DIR}/include)

#_______________________ include modular files _______________________

include(cmake/common_sources/common_sources.cmake)
include(cmake/common_test_sources/common_test_sources.cmake)
include(cmake/compiler_options/compiler_options.cmake)
include(cmake/libsndfile/libsndfile.cmake)
include(cmake/ffmpeg/ffmpeg.cmake)
include_directories(${FFMPEG_INCLUDE_DIRS})

include(src/dsp/echo/echo.cmake)
include(src/dsp/gain/gain.cmake)
include(src/dsp/fade/fade.cmake)
include(src/dsp/normalization/norm.cmake)

# Create the engine library
add_library(aj_audio_engine
    src/core/aj_audio_engine.cc
)

# Link dependencies to the engine library
target_link_libraries(aj_audio_engine
    PRIVATE echo_avx echo_sse gain_avx fade_avx norm_gain_avx ${SNDFILE_LIBRARY} ${FFMPEG_LIBRARIES}
)

target_include_directories(aj_audio_engine PRIVATE ${FFMPEG_INCLUDE_DIRS})



# main executable
add_executable(sound_processor
    src/main.cc
    ${COMMON_SOURCES}
)

target_include_directories(sound_processor PRIVATE ${FFMPEG_INCLUDE_DIRS})


# Link all libraries to main executable
target_link_libraries(sound_processor
    PRIVATE aj_audio_engine ${SNDFILE_LIBRARY} ${FFMPEG_LIBRARIES}
)


# tests main executable
add_executable(test
    test/test.cc
    ${COMMON_TEST_SOURCES}
    ${COMMON_SOURCES}
)

target_include_directories(test PRIVATE ${FFMPEG_INCLUDE_DIRS})

# Link all libraries to test executable
target_link_libraries(test
    PRIVATE echo_avx echo_sse gain_avx fade_avx norm_gain_avx ${SNDFILE_LIBRARY} ${FFMPEG_LIBRARIES}
)