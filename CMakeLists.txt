cmake_minimum_required(VERSION 3.13)
project(AJ_Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the output directory for executables to "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Find libsndfile
find_library(SNDFILE_LIBRARY sndfile REQUIRED)
find_path(SNDFILE_INCLUDE_DIR sndfile.h REQUIRED)
include_directories(${SNDFILE_INCLUDE_DIR})


# Common source files (without main or benchmark)
set(COMMON_SOURCES
    src/file_io/wav_file.cc
    src/file_io/audio_file.cc
    src/dsp/echo/echo.cc
)

# Echo AVX 512 Sources
add_library(
    echo_avx512 STATIC src/dsp/echo/echo_avx512.cc
)
target_compile_options(
    echo_avx512 PRIVATE -mavx512f
)

# Echo AVX Sources
add_library(
    echo_avx STATIC src/dsp/echo/echo_avx.cc
)
target_compile_options(
    echo_avx PRIVATE -mavx
)

# Echo SSE Sources
add_library(
    echo_sse STATIC src/dsp/echo/echo_sse.cc
)
target_compile_options(
    echo_sse PRIVATE -msse4.1
)

# compiler scalar optimizations 
add_compile_options(-O2)

# main executable
add_executable(sound_processor
    src/main.cc
    ${COMMON_SOURCES}
)

# Link all libraries to main executable
target_link_libraries(sound_processor
    PRIVATE echo_avx512 echo_avx echo_sse ${SNDFILE_LIBRARY}
)


# Common test sources
set(COMMON_TEST_SOURCES
    test/test_read_wav.cc
)

# tests main executable
add_executable(test
    test/test.cc
    ${COMMON_TEST_SOURCES}
    ${COMMON_SOURCES}
)

# Link all libraries to test executable
target_link_libraries(test
    PRIVATE echo_avx512 echo_avx echo_sse ${SNDFILE_LIBRARY}
)