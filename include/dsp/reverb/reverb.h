#pragma once
#include <iostream>
#include <memory>
#include <algorithm>

#include "dsp/effect.h"
#include "core/types.h"
#include "dsp/reverb/all_pass_filter.h"
#include "dsp/reverb/comb_filter.h"

namespace AJ::dsp::reverb {

class ReverbParams : public EffectParams {
public:
    float mDelayMS;
    float mWetMix;
    float mDryMix;
    int mSamplerate;
    float mGain;

    ~ReverbParams() override = default;
};

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

    std::shared_ptr<ReverbParams> mParams;

    CombFilters mCombFilters;
    AllPassFilters mAllPassFilters;

    bool checkValidIndxes(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler);

public:
    Reverb(){
        mParams = std::make_shared<ReverbParams>();

        mParams->mGain = REVERB_GAIN;
        mParams->mDelayMS = REVERB_DELAY;
        mParams->mDryMix = REVERB_DRY_MIX;
        mParams->mWetMix = REVERB_WET_MIX;

        for(auto &comb : mCombFilters){
            comb = std::make_unique<CombFilter>();
        }

        for(auto &all_pass : mAllPassFilters){
            all_pass = std::make_unique<AllPassFilter>(mParams->mSamplerate);
        }
    }

    Reverb(int samplerate){
        mParams = std::make_shared<ReverbParams>();
        mParams->mSamplerate = samplerate;
        mParams->mGain = REVERB_GAIN;
        mParams->mDelayMS = REVERB_DELAY;
        mParams->mDryMix = REVERB_DRY_MIX;
        mParams->mWetMix = REVERB_WET_MIX;

        for(auto &comb : mCombFilters){
            comb = std::make_unique<CombFilter>();
        }

        for(auto &all_pass : mAllPassFilters){
            all_pass = std::make_unique<AllPassFilter>(mParams->mSamplerate);
        }
    }

    void setDelay(float delayMS){
        mParams->mDelayMS = std::clamp(delayMS, 
            static_cast<float>(REVERB_DELAY_MIN),
            static_cast<float>(REVERB_DELAY_MAX)
        );
    }

    void setDryMix(float mix){
        mParams->mDryMix = std::clamp(mix, 
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX)
        );
    }

    void setWetMix(float mix){
        mParams->mWetMix = std::clamp(mix, 
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX)
        );
    }

    void setGain(float gain){
        mParams->mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    void setSamplerate(int samplerate){
        mParams->mSamplerate = samplerate;
    }

    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    bool process(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler) override;
};

};