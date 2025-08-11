# gain AVX Sources
add_library(
    norm_gain_avx STATIC src/dsp/normalization/gain_avx.cc
)

target_compile_options(
    norm_gain_avx PRIVATE -mavx
)
