# gain AVX Sources
add_library(
    fade_avx STATIC src/dsp/fade/fade_avx.cc
)

target_compile_options(
    fade_avx PRIVATE -mavx
)
