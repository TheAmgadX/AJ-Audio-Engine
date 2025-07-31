# compiler optimizations 
if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    add_compile_options(-O0 -g)
else()
    add_compile_options(-O3 -march=native -mfpmath=sse)
endif()
