#include <iostream>

#include "dsp/gain.h"
#include "core/types.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
*/

bool AJ::dsp::Gain::gainAVX(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(end < start || start < 0 || start >= buffer.size() || end >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    __m256 gain = _mm256_set1_ps(mParams->mGain);
    __m256 max_val = _mm256_set1_ps(1.0f);
    __m256 min_val = _mm256_set1_ps(-1.0f);

    sample_pos i;
    for(i = start; i + 7 <= end; i += 8){
        __m256 samples = _mm256_mul_ps( // multiply current samples by gain value
            _mm256_loadu_ps(&buffer[i]), // laod 8 current samples
            gain
        );

        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 8 samples in the buffer
        _mm256_storeu_ps(
            &buffer[i],
             _mm256_max_ps (
                _mm256_min_ps(samples, max_val),
                min_val
            )
        );
    }

    // calculate the rest if there.
    for(i; i <= end; ++i){
        calculate_gain_sample(buffer[i]);
    }

    return true;
}


