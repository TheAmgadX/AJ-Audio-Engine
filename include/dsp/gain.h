#pragma once
#include <iostream>

#include "effect.h"
#include "core/types.h"

namespace AJ::dsp {
class Gain : public AJ::dsp::Effect {

private:
    gain_t mGain;
    void calculate_gain_sample(sample_t &sample);
    void gainNaive(Float &buffer, sample_pos start, sample_pos end);
    void gainAVX(Float &buffer, sample_pos start, sample_pos end);
public:
    bool setGain(gain_t gain){
        if(gain < 0.0 || gain > 2.0){
        std::cerr << "Invalid gain: " << gain << ". Gain must be in [0.0, 2.0]\n";
            return false;
        }
        mGain = gain;
        return true;
    }

    gain_t gain(){
        return mGain;
    }

    void process(Float &buffer, sample_pos start, sample_pos end) override;
    
};
};