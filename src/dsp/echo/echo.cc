#include <iostream>
#include <limits>
#include <thread>

#include "dsp/echo.h"
#include "core/types.h"

void AJ::dsp::Echo::process(Float &buffer, sample_pos start, sample_pos end) {
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        echoSIMD_AVX(buffer, start, end);
    }
#endif

#if defined(__SSE__)
    if (__builtin_cpu_supports("sse")) {
        echoSIMD_SSE(buffer, start, end);
    }
#endif

    echoNaive(buffer, start, end);
}

AJ::sample_t AJ::dsp::Echo::calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx){
    if(sample_idx >= in.size() || sample_idx < 0){
        std::cerr << "Invalid Index: sample_idx is not in a valid range.\n";
        return 0.0f;
    }
    if(echo_idx >= in.size() || echo_idx < 0){
        std::cerr << "Invalid Index: echo_idx is not in a valid range.\n";
        return 0.0f;
    }

    sample_t newSample = in[sample_idx] + (in[echo_idx] * mDecay);

    if(newSample > 1.0f) newSample = 1.0f;
    else if (newSample < -1.0f) newSample = -1.0f;

    return newSample;
}

void AJ::dsp::Echo::SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate){
    mDelaySamples = sampleRate * delayInSeconds;
}

void AJ::dsp::Echo::echoNaive(Float &buffer, sample_pos start, sample_pos end){
    /*
        ?Echo Effect Equation:
            Echo_Sample = in[i - _mDelaySamples] * _mDecay
            out[i] = in[i] + Echo_Sample

        ! we must avoid writing at the input buffer to avoid speculative samples.
    */

    // check valid indexes ranges
    if(end < start || start < 0 || start + mDelaySamples >= buffer.size() || end >= buffer.size()){
        std::cerr << "invalid indexes for echo effect.\n";
        return;
    }

    Float out;
    //* copy the first samples
    out.resize(end - start + 1, 0.0f);
    std::copy(buffer.begin() + start, buffer.begin() + start + mDelaySamples, out.begin());

    for(sample_pos i = start + mDelaySamples; i <= end; ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mDelaySamples);
        out[i - start] = sample;
    }

    //* copy the new samples into the main buffer
    std::copy(out.begin(), out.end(), buffer.begin() + start);
}



