#include <iostream>
#include <limits>
#include <thread>

#include "echo.h"
#include "types.h"

// SIMD Headers:
#include <xmmintrin.h> // SSE (Streaming SIMD Extensions) - 128-bit operations on 4 floats
#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
    -  AVX-512 - 512-bit operations (16 floats)
*/


// TODO: refactor the function to support the audio blocks structure 
void AJ::dsp::Echo::process(AudioBufferBlocks &buffer, sample_pos start, sample_pos end) {
#if defined(__AVX512F__)
    if (__builtin_cpu_supports("avx512f")) {
        EchoSIMD_AVX512();
        return;
    }
#endif

#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        EchoSIMD_AVX();
        return;
    }
#endif

#if defined(__SSE__)
    if (__builtin_cpu_supports("sse")) {
    if (buffer.size() == 2){
        // stereo
        EchoSIMD_SSE(buffer[0], start, end);
        EchoSIMD_SSE(buffer[1], start, end);
    } else {
        // mono
        EchoSIMD_SSE(buffer[0], start, end);
    }
        return;
    }
#endif

    // Naive Function
    if (buffer.size() == 2){
        // stereo
        EchoNaive(buffer[0], start, end);
        EchoNaive(buffer[1], start, end);
    } else {
        // mono
        EchoNaive(buffer[0], start, end);
    }
}

// TODO: refactor the function to support the audio blocks structure 
AJ::sample_t AJ::dsp::Echo::CalculateNewSampleWithEcho(Float &in, sample_pos sample_idx, sample_pos echo_idx){
    if(sample_idx >= in.size() || sample_idx < 0){
        std::cerr << "Invalid Index: sample_idx is not in a valid range.\n";
        return 0.0f;
    }
    if(echo_idx >= in.size() || echo_idx < 0){
        std::cerr << "Invalid Index: echo_idx is not in a valid range.\n";
        return 0.0f;
    }

    sample_t newSample = in[sample_idx]  + (in[echo_idx] * _mDecay);

    if(newSample > 1.0f) newSample = 1.0f;
    else if (newSample < -1.0f) newSample = -1.0f;

    return newSample;
}

void AJ::dsp::Echo::SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate){
    _mDelaySamples = sampleRate * delayInSeconds;
}

// TODO: refactor the function to support the audio blocks structure 
void AJ::dsp::Echo::EchoNaive(AudioChannelBufferBlocks &blocks, sample_pos start, sample_pos end){
    /*
        ?Echo Effect Equation:
            Echo_Sample = in[i - _mDelaySamples] * _mDecay
            out[i] = in[i] + Echo_Sample

        ! we must avoid writing at the input buffer to avoid speculative samples problem.
    */

    // check valid indexes ranges
    if(end < start || start < 0 || start + _mDelaySamples >= in.size() || end >= in.size()){
        std::cerr << "invalid indexes for echo effect.\n";
        return;
    }

    Float out;
    //* copy the first samples
    out.resize(end - start + 1, 0.0f);
    std::copy(in.begin() + start, in.begin() + start + _mDelaySamples, out.begin());

    for(sample_pos i = start + _mDelaySamples; i <= end; ++i){
        sample_t sample = CalculateNewSampleWithEcho(in, i, i - _mDelaySamples);
        out[i - start] = sample;
    }

    //* copy the new samples into the main buffer
    std::copy(out.begin(), out.end(), in.begin() + start);
}



