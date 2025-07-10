cmake_minimum_required(VERSION 3.13)
project(AJ_Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the output directory for executables to "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Find libsndfile
find_package(SndFile REQUIRED)

# Common source files (without main or benchmark)
set(COMMON_SOURCES
    src/io/WAV_File.cc
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
    src/main.cpp
    ${COMMON_SOURCES}
)

# Link all libraries to main executable
target_link_libraries(sound_processor
    PRIVATE echo_avx512 echo_avx echo_sse SndFile::sndfile
)

# tests executable
add_executable(test
    test/test.cpp
    ${COMMON_SOURCES}
)

# Link all libraries to test executable
target_link_libraries(test
    PRIVATE echo_avx512 echo_avx echo_sse SndFile::sndfile
)
