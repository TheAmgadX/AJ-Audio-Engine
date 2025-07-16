# gain AVX Sources
add_library(
    gain_avx STATIC src/dsp/gain/gain_avx.cc
)

target_compile_options(
    gain_avx PRIVATE -mavx
)
