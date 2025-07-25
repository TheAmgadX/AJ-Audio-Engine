#pragma once
#include <iostream>
#include <algorithm>

#include "core/types.h"

namespace AJ::dsp::reverb {
class AllPassFilter {

private:
    float mDelayMS;
    sample_c mDelay;
    int mSamplerate;
    float mGain;
public:
    AllPassFilter() {
        mDelayMS = 89.27f;
        mGain = 0.131f;
        mSamplerate = 44100;
        mDelay = mDelayMS * (float(mSamplerate) / 1000.0f);
    }

    AllPassFilter(int samplerate) {
        mDelayMS = 89.27f; 
        mGain = 0.131f;
        mSamplerate = samplerate;
        mDelay = mDelayMS * (float(mSamplerate) / 1000.0f);
    }

    void setDelay(float delayMS, const sample_c samplerate){
        mDelay = (float(samplerate) / 1000) * delayMS;
    }
    
    void setGain(float gain){
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    void setSamplerate(int samplerate){
        mSamplerate = std::clamp(samplerate, 8100, 44100);
        setDelay(mDelayMS, samplerate);
    }

    Float process(Float &input);
};

};