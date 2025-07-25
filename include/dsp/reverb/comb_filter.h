#pragma once
#include <iostream>
#include <algorithm>

#include "core/types.h"

namespace AJ::dsp::reverb {
class CombFilter {
private:
    sample_c mDelay;
    float mGain;
public:
    bool setDelay(float delayMS, const int samplerate, const sample_c size){
        if(delayMS <= 0) {
            mDelay = (REVERB_DELAY / 1000) * samplerate; // 78.9ms 
            return false;
        }

        mDelay = (delayMS / 1000) * samplerate;

        if(mDelay >= size){
            std::cerr << "invalid delay: delay is longer than the buffer size\n"; 
            return false;
        }
        return true;
    }
    
    void setGain(float gain){
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    sample_t process(Float &input, Float &output, sample_pos idx, sample_pos start);
};

};