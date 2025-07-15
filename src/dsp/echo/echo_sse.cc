#include <iostream>

#include "dsp/echo.h"
#include "core/types.h"

// SIMD Headers:
#include <xmmintrin.h> // SSE (Streaming SIMD Extensions) - 128-bit operations on 4 floats

void AJ::dsp::Echo::echoSIMD_SSE(Float &buffer, sample_pos start, sample_pos end){
    // check valid indexes ranges
    if(end < start || start < 0 || start + mDelaySamples >= buffer.size() || end >= buffer.size()){
        std::cerr << "invalid indexes for echo effect.\n";
        return;
    }

    Float out;
    //* copy the first samples
    out.resize(end - start + 1, 0.0f);
    std::copy(buffer.begin() + start, buffer.begin() + start + mDelaySamples, out.begin());

    size_t i;
    // set SSE vector for decay value
    __m128 decay_v = _mm_set1_ps(mDecay);

    // set max and min values for clamping
    __m128 max_val = _mm_set1_ps(1.0f);
    __m128 min_val = _mm_set1_ps(-1.0f);

    for(i = start + mDelaySamples; i + 3 <= end; i += 4){
        // load 4 current samples in SSE vector
        // load 4 delayed samples in SSE vector
        // echo_samples = delay_samples * decay_v
        // new_samples = echo_samples + samples
        __m128 new_samples = _mm_add_ps(
            _mm_mul_ps(
                _mm_load_ps(&buffer[i - mDelaySamples]), // 4 delayed samples
                decay_v 
            ), // multiply delayed_samples by decay factor
            _mm_load_ps(&buffer[i]) // 4 current samples
        ); // add echo samples with current samples
        
        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 4 samples in the proper indexes of out vector 
        _mm_store_ps(
            &out[i - start],
             _mm_max_ps (
                _mm_min_ps(new_samples, max_val),
                min_val
            )
        );
    }

    // calculate the rest if there.
    for(i; i <= end; ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mDelaySamples);
        out[i - start] = sample;
    }

    std::copy(out.begin(), out.end(), buffer.begin() + start);
}

