#include <iostream>
#include "core/types.h"

#include "dsp/effect.h"
#include "dsp/gain.h"

void AJ::dsp::Gain::process(Float &buffer, sample_pos start, sample_pos end){

    if(mGain == 1.0) return;

#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
    gainAVX(buffer, start, end);
        return;
    }
#endif
    gainNaive(buffer, start, end);
}

void AJ::dsp::Gain::calculate_gain_sample(sample_t &sample){
    sample *= mGain;

    if(sample < -1.0){
        sample = -1.0;
    }

    if(sample > 1.0){
        sample = 1.0;
    }
}

void AJ::dsp::Gain::gainNaive(Float &buffer, sample_pos start, sample_pos end){
    // check valid indexes ranges
    if(end < start || start < 0 || start >= buffer.size() || end >= buffer.size()){
        std::cerr << "invalid indexes for gain effect.\n";
        return;
    }

    for(sample_pos i = start; i <= end; ++i){
        calculate_gain_sample(buffer[i]);
    }
}