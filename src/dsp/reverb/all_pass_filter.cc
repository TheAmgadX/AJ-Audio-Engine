#include <iostream>
#include <cmath>
#include "core/types.h"

#include "dsp/reverb/all_pass_filter.h"


AJ::Float AJ::dsp::reverb::AllPassFilter::process(Float &input){
    Float out;
    size_t size = input.size();
    out.resize(size);

    for(size_t i = 0; i < size; ++i){
        if(i >= mDelay) {
            // All-pass filter formula: y[n] = -g·x[n] + x[n-M] + g·y[n-M]
            out[i] = (-mGain * input[i]) + 
                     input[i - mDelay] + 
                     (mGain * out[i - mDelay]);
        } else {
            out[i] = input[i];
        }
    }

    return out;
}