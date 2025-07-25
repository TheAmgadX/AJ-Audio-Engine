#pragma once
#include <iostream>
#include <memory>
#include <algorithm>

#include "dsp/effect.h"
#include "core/types.h"
#include "dsp/reverb/all_pass_filter.h"
#include "dsp/reverb/comb_filter.h"

namespace AJ::dsp::reverb {


using CombFilters = std::array<std::unique_ptr<CombFilter>, kCombFilters>;
using AllPassFilters = std::array<std::unique_ptr<AllPassFilter>, kAllPassFilters>;

// TODO: add different presets like hall, room, cathedral, ..etc 
/*
    Reverb Type	    Delay (mDelayMS)	Wet Mix	    Dry Mix	    Gain (mGain)
    Room	         25.0f	              0.3f	      0.7f	        0.4f	
    Hall	         70.0f	              0.5f	      0.5f	        0.7f	
    Cathedral	     110.0f	              0.8f	      0.2f	        0.9f	
*/
// TODO:: add docs to how to use and set the the reverb for APIs Docs.
class Reverb : public AJ::dsp::Effect {

private:
    float mDelayMS;
    float mWetMix;
    float mDryMix;
    int mSamplerate;
    float mGain;

    CombFilters mCombFilters;
    AllPassFilters mAllPassFilters;

    bool checkValidIndxes(Float &buffer, sample_pos start, sample_pos end);

public:
    Reverb(int samplerate){
        mSamplerate = samplerate;
        mGain = REVERB_GAIN;
        mDelayMS = REVERB_DELAY;
        mDryMix = REVERB_DRY_MIX;
        mWetMix = REVERB_WET_MIX;

        for(auto &comb : mCombFilters){
            comb = std::make_unique<CombFilter>();
        }

        for(auto &all_pass : mAllPassFilters){
            all_pass = std::make_unique<AllPassFilter>(mSamplerate);
        }
    }

    void setDelay(float delayMS){
        mDelayMS = std::clamp(delayMS, 
            static_cast<float>(REVERB_DELAY_MIN),
            static_cast<float>(REVERB_DELAY_MAX)
        );
    }

    void setDryMix(float mix){
        mDryMix = std::clamp(mix, 
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX)
        );
    }

    void setWetMix(float mix){
        mWetMix = std::clamp(mix, 
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX)
        );
    }

    void setGain(float gain){
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    void process(Float &buffer, sample_pos start, sample_pos end) override;
};

};