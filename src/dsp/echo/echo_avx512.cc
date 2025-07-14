#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
    -  AVX-512 - 512-bit operations (16 floats)
*/
void AJ::dsp::Echo::echoSIMD_AVX512(Float &buffer, sample_pos start, sample_pos end){

}