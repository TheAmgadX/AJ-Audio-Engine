#include <iostream>
#include "core/types.h"

#include "dsp/effect.h"
#include "dsp/gain.h"
#include "core/effect_params.h"

bool AJ::dsp::Gain::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) {
    std::shared_ptr<GainParams> gainParams = std::dynamic_pointer_cast<GainParams>(params);
    // if gainParams is nullptr that means it's not a shared_ptr of GainParams
    if(!gainParams){
        const std::string message = "Effect parameters must be of type GainParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = gainParams; 
    return true;
}

bool AJ::dsp::Gain::process(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler){

    if(mParams->mGain == 1.0) return false;

#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return gainAVX(buffer, start, end, handler);

    }
#endif
    return gainNaive(buffer, start, end, handler);
}

void AJ::dsp::Gain::calculate_gain_sample(sample_t &sample){
    sample *= mParams->mGain;

    if(sample < -1.0){
        sample = -1.0;
    }

    if(sample > 1.0){
        sample = 1.0;
    }
}

bool AJ::dsp::Gain::gainNaive(Float &buffer, sample_pos start, sample_pos end, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(end < start || start < 0 || start >= buffer.size() || end >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    for(sample_pos i = start; i <= end; ++i){
        calculate_gain_sample(buffer[i]);
    }

    return true;
}