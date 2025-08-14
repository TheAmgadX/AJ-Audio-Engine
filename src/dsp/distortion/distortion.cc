#include <iostream>
#include <cmath>

#include "dsp/distortion.h"
#include "core/types.h"
#include "core/error_handler.h"

std::shared_ptr<AJ::dsp::distortion::DistortionParams> AJ::dsp::distortion::DistortionParams::
    create(Params &params, AJ::error::IErrorHandler &handler) {
    std::shared_ptr<DistortionParams> distortionParams = std::make_shared<DistortionParams>(PrivateTag{});


    if(params.mStart > params.mEnd || params.mStart < 0) {
        const std::string message = "invalid range indexes parameters for distortion effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    distortionParams->setGain(params.mGain);
    distortionParams->setType(params.mType);
    distortionParams->setStart(params.mStart);
    distortionParams->setEnd(params.mEnd);

    return distortionParams;
}

bool AJ::dsp::distortion::Distortion::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
    std::shared_ptr<DistortionParams> distParams = std::dynamic_pointer_cast<DistortionParams>(params);
    // if distParams is nullptr that means it's not a shared_ptr of DistortionParams
    if(!distParams){
        const std::string message = "Effect parameters must be of type DistortionParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = distParams; 
    return true;
}

bool AJ::dsp::distortion::Distortion::process(Float &buffer, AJ::error::IErrorHandler &handler) {
    switch (mParams->Type())
    {
    case DistortionType::SoftClipping:
        return softClipping(buffer, handler);
    default:
        return false;
    }
}

bool AJ::dsp::distortion::Distortion::softClipping(Float &buffer, AJ::error::IErrorHandler &handler) {
    // check valid indexes ranges
    sample_c start = mParams->Start();
    sample_c end = mParams->End();

    if(end < start || start < 0 || start >= buffer.size() || end >= buffer.size()){
        const std::string message = "invalid indexes for distortion effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    float gain = mParams->Gain();

    if(gain == 0){
        const std::string message = "invalid gain value for distortion effect it must be bigger than 0.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    float gain_tanh = 1 / std::tanh(gain);

    for(size_t i = mParams->Start(); i <= end; ++i){
        buffer[i] = std::tanh(gain * buffer[i]) * gain_tanh;
    }

    return true;
}

