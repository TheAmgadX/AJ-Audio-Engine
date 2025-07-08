cmake_minimum_required(VERSION 3.13)
project(AJ_Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the output directory for executables to "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include
    # ${PROJECT_SOURCE_DIR}/include/file_io
    # ${PROJECT_SOURCE_DIR}/include/dsp
    # ${PROJECT_SOURCE_DIR}/include/block_files_system
    # ${PROJECT_SOURCE_DIR}/include/undo_system
    # ${PROJECT_SOURCE_DIR}/include/audio_io
)

# Common source files (without main or benchmark)
set(COMMON_SOURCES
    # src/xxx.cpp the cpp files that have the implementation but not .exe files
    src/io/WAV_File.cpp
)

# for SIMD instructions support
add_compile_options(-mavx512f -mavx -msse4.1 -mfpmath=sse -fno-fast-math)


# main executable
add_executable(sound_processor
    src/main.cpp
    ${COMMON_SOURCES}
)

# benchmark executable
add_executable(test
    test/test.cpp
    ${COMMON_SOURCES}
)