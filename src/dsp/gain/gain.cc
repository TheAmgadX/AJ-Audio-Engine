#include <iostream>
#include <memory>
#include "core/types.h"
#include "core/errors.h"

#include "dsp/effect.h"
#include "dsp/gain.h"
#include "core/effect_params.h"

std::shared_ptr<AJ::dsp::gain::GainParams> AJ::dsp::gain::GainParams::create(Params& params, AJ::error::IErrorHandler &handler){
    
    std::shared_ptr<GainParams> gainParams = std::make_shared<GainParams>(PrivateTag{});
        
    if(params.mStart > params.mEnd || params.mStart < 0) {
        const std::string message = "invalid range indexes parameters for gain effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    if(params.mGain < 0.0f || params.mGain > 5.0f){
        const std::string message = "invalid gain value for gain effect, gain must be in range [0.0f, 5.0f].\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    gainParams->setGain(params.mGain);
    gainParams->setStart(params.mStart);
    gainParams->setEnd(params.mEnd);

    return gainParams;
}

bool AJ::dsp::gain::Gain::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) {
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

bool AJ::dsp::gain::Gain::process(Float &buffer, AJ::error::IErrorHandler &handler){

    if(mParams->Gain() == 1.0) return false;

#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return gainAVX(buffer, handler);

    }
#endif
    return gainNaive(buffer, handler);
}

void AJ::dsp::gain::Gain::calculate_gain_sample(sample_t &sample){
    sample *= mParams->Gain();

    if(sample < -1.0){
        sample = -1.0;
    }

    if(sample > 1.0){
        sample = 1.0;
    }
}

bool AJ::dsp::gain::Gain::gainNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 || 
        mParams->Start() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    for(sample_pos i = mParams->Start(); i <= mParams->End(); ++i){
        calculate_gain_sample(buffer[i]);
    }

    return true;
}