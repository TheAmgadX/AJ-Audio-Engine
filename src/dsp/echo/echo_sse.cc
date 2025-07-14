#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"

// SIMD Headers:
#include <xmmintrin.h> // SSE (Streaming SIMD Extensions) - 128-bit operations on 4 floats

void AJ::dsp::Echo::echoSIMD_SSE(Float &buffer, sample_pos start, sample_pos end){

}