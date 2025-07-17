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

include(src/dsp/echo/echo.cmake)
include(src/dsp/gain/gain.cmake)


# main executable
add_executable(sound_processor
    src/main.cc
    ${COMMON_SOURCES}
)

# Link all libraries to main executable
target_link_libraries(sound_processor
    PRIVATE echo_avx echo_sse gain_avx ${SNDFILE_LIBRARY}
)

# tests main executable
add_executable(test
    test/test.cc
    ${COMMON_TEST_SOURCES}
    ${COMMON_SOURCES}
)

# Link all libraries to test executable
target_link_libraries(test
    PRIVATE echo_avx echo_sse gain_avx ${SNDFILE_LIBRARY}
)