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