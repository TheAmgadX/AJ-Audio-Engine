#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"
#include "core/error_handler.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
    -  AVX-512 - 512-bit operations (16 floats)
*/


bool AJ::dsp::Echo::echoSIMD_AVX(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->mEnd < mParams->mStart || mParams->mStart < 0 || mParams->mStart + 
        mParams->mDelaySamples >= buffer.size() || mParams->mEnd >= buffer.size()){
        const std::string message = "invalid indexes for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    Float out;
    //* copy the first samples
    out.resize(mParams->mEnd - mParams->mStart + 1, 0.0f);
    std::copy(buffer.begin() + mParams->mStart, buffer.begin() + mParams->mStart + mParams->mDelaySamples, out.begin());

    size_t i;
    // set SSE vector for decay value
    __m256 decay_v = _mm256_set1_ps(mParams->mDecay);

    // set max and min values for clamping
    __m256 max_val = _mm256_set1_ps(1.0f);
    __m256 min_val = _mm256_set1_ps(-1.0f);

    for(i = mParams->mStart + mParams->mDelaySamples; i + 7 <= mParams->mEnd; i += 8){
        // load 8 current samples in SSE vector
        // load 8 delayed samples in SSE vector
        // echo_samples = delay_samples * decay_v
        // new_samples = echo_samples + samples
        __m256 new_samples = _mm256_add_ps(
            _mm256_mul_ps(
                _mm256_loadu_ps(&buffer[i - mParams->mDelaySamples]), // 8 delayed samples
                decay_v 
            ), // multiply delayed_samples by decay factor
            _mm256_loadu_ps(&buffer[i]) // 8 current samples
        ); // add echo samples with current samples
        
        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 8 samples in the proper indexes of out vector 
        _mm256_storeu_ps(
            &out[i - mParams->mStart],
             _mm256_max_ps (
                _mm256_min_ps(new_samples, max_val),
                min_val
            )
        );
    }

    // calculate the rest if there.
    for(i; i <= mParams->mEnd; ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mParams->mDelaySamples, handler);
        out[i - mParams->mStart] = sample;
    }

    std::copy(out.begin(), out.end(), buffer.begin() + mParams->mStart);
    return true;
}
