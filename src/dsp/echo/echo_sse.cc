#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"
#include "core/error_handler.h"

// SIMD Headers:
#include <xmmintrin.h> // SSE (Streaming SIMD Extensions) - 128-bit operations on 4 floats

bool AJ::dsp::echo::Echo::echoSIMD_SSE(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 ||
        mParams->Start() + mParams->DelaySamples() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    Float out;
    //* copy the first samples
    out.resize(mParams->End() - mParams->Start() + 1, 0.0f);
    std::copy(buffer.begin() + mParams->Start(),
        buffer.begin() + mParams->Start() + mParams->DelaySamples(), out.begin());

    size_t i;
    // set SSE vector for decay value
    __m128 decay_v = _mm_set1_ps(mParams->Decay());

    // set max and min values for clamping
    __m128 max_val = _mm_set1_ps(1.0f);
    __m128 min_val = _mm_set1_ps(-1.0f);

    for(i = mParams->Start() + mParams->DelaySamples(); i + 3 <= mParams->End(); i += 4){
        // load 4 current samples in SSE vector
        // load 4 delayed samples in SSE vector
        // echo_samples = delay_samples * decay_v
        // new_samples = echo_samples + samples
        __m128 new_samples = _mm_add_ps(
            _mm_mul_ps(
                _mm_load_ps(&buffer[i - mParams->DelaySamples()]), // 4 delayed samples
                decay_v 
            ), // multiply delayed_samples by decay factor
            _mm_load_ps(&buffer[i]) // 4 current samples
        ); // add echo samples with current samples
        
        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 4 samples in the proper indexes of out vector 
        _mm_store_ps(
            &out[i - mParams->Start()],
             _mm_max_ps (
                _mm_min_ps(new_samples, max_val),
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

