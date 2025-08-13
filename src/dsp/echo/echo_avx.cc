#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"
#include "core/error_handler.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
    -  AVX-512 - 512-bit operations (16 floats)
*/


bool AJ::dsp::echo::Echo::echoSIMD_AVX(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 || mParams->Start() + 
        mParams->DelaySamples() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    Float out;
    //* copy the first samples
    out.resize(mParams->End() - mParams->Start() + 1, 0.0f);
    std::copy(buffer.begin() + mParams->Start(), buffer.begin() + mParams->Start() + mParams->DelaySamples(), out.begin());

    size_t i;
    // set SSE vector for decay value
    __m256 decay_v = _mm256_set1_ps(mParams->Decay());

    // set max and min values for clamping
    __m256 max_val = _mm256_set1_ps(1.0f);
    __m256 min_val = _mm256_set1_ps(-1.0f);

    for(i = mParams->Start() + mParams->DelaySamples(); i + 7 <= mParams->End(); i += 8){
        // load 8 current samples in SSE vector
        // load 8 delayed samples in SSE vector
        // echo_samples = delay_samples * decay_v
        // new_samples = echo_samples + samples
        __m256 new_samples = _mm256_add_ps(
            _mm256_mul_ps(
                _mm256_loadu_ps(&buffer[i - mParams->DelaySamples()]), // 8 delayed samples
                decay_v 
            ), // multiply delayed_samples by decay factor
            _mm256_loadu_ps(&buffer[i]) // 8 current samples
        ); // add echo samples with current samples
        
        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 8 samples in the proper indexes of out vector 
        _mm256_storeu_ps(
            &out[i - mParams->Start()],
             _mm256_max_ps (
                _mm256_min_ps(new_samples, max_val),
                min_val
            )
        );
    }

    // calculate the rest if there.
    for(i; i <= mParams->End(); ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mParams->DelaySamples(), handler);
        out[i - mParams->Start()] = sample;
    }

    std::copy(out.begin(), out.end(), buffer.begin() + mParams->Start());
    return true;
}
