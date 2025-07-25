#include <iostream>
#include <cmath>
#include "core/types.h"

#include "dsp/reverb/comb_filter.h"


AJ::sample_t AJ::dsp::reverb::CombFilter::process(Float &input, Float &output, sample_pos i, sample_pos start){
    sample_t sample = 0.0f;
    sample_pos j = i - start - mDelay;
    sample += input[i];
    if(j >= 0){
        sample += mGain * output[j];
    }

    output[j + mDelay] = sample;

    return sample;
}