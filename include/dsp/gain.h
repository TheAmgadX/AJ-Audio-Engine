#pragma once
#include <iostream>

#include "effect.h"
#include "core/types.h"
#include "core/error_handler.h"

namespace AJ::dsp {

class GainParams : public EffectParams {
public:
    gain_t mGain;
    ~GainParams() override = default;
};

class Gain : public AJ::dsp::Effect {

private:
    std::shared_ptr<GainParams> mParams;
    void calculate_gain_sample(sample_t &sample);
    bool gainNaive(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler);
    bool gainAVX(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler);
public:
    bool setGain(gain_t gain, AJ::error::IErrorHandler &handler){
        if(gain < 0.0 || gain > 2.0){
            const std::string message = "Invalid gain: " + std::to_string(gain) 
            + " Gain must be in range of [0.0, 2.0]\n";

            handler.onError(error::Error::InvalidEffectParameters, message);
            return false;
        }
        mParams->mGain = gain;
        return true;
    }

    Gain(){
        mParams = std::make_shared<GainParams>();
        mParams->mGain = 1;
    }

    gain_t gain(){
        return mParams->mGain;
    }

    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    bool process(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler) override;
    
};
};