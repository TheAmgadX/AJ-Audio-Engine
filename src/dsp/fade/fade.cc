#include <iostream>
#include <algorithm>

#include "dsp/fade.h"
#include "core/types.h"
#include "core/error_handler.h"

std::shared_ptr<AJ::dsp::fade::FadeParams> AJ::dsp::fade::FadeParams::create(Params& params,
    AJ::error::IErrorHandler &handler){
    if(params.mLowGain > params.mHighGain){
        const std::string message = "invalid gain parameters for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    } 

    if(params.mStart > params.mEnd || params.mStart < 0){
        const std::string message = "invalid range indexes for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }


    std::shared_ptr<FadeParams> fadeParams = std::make_shared<FadeParams>(PrivateTag{});

    fadeParams->mLowGain = std::clamp(params.mLowGain, 0.0f, 2.0f);
    fadeParams->mHighGain = std::clamp(params.mHighGain, 0.0f, 2.0f);
    fadeParams->setStart(params.mStart);
    fadeParams->setEnd(params.mEnd);
    fadeParams->mMode = params.mMode;

    return fadeParams;
}

bool AJ::dsp::fade::Fade::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
    std::shared_ptr<FadeParams> fadeParams = std::dynamic_pointer_cast<FadeParams>(params);
    // if fadeParams is nullptr that means it's not a shared_ptr of FadeParams
    if(!fadeParams){
        const std::string message = "Effect parameters must be of type FadeParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = fadeParams; 
    return true;
}

bool AJ::dsp::fade::Fade::process(Float &buffer, AJ::error::IErrorHandler &handler){
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return fadeAVX(buffer, handler);
    }
#endif

    return fadeNaive(buffer, handler);
}

bool AJ::dsp::fade::Fade::fadeNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    /**
     * Fade equation:
     *   output[i] = input[i] * currentGain
     *
     * currentGain starts at lowGain if fade in and highGain if fade out
     * and increases/decreases by gainStep for each sample.
     *
     * gainDifference = highGain - lowGain
     * totalSamples   = End() - Start()
     * gainStep       = gainDifference / totalSamples
     * 
     * gainStep *= -1 if it's fade out.
     * 
     * currentGain = highGain if fade out
     * currentGain = lowGain if fade in
     */


    // check valid indexes range.
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return false;
    }

    float currentGain;

    float gainDiff = mParams->highGain() - mParams->lowGain();
    sample_c totalSamples = mParams->End() - mParams->Start() + 1;
    double gainStep = gainDiff / totalSamples;

    if(mParams->mode() == FadeMode::In){
        currentGain = mParams->lowGain();
    } else {
        currentGain = mParams->highGain();
        gainStep *= -1;
    }


    for(size_t i = mParams->Start(); i <= mParams->End(); ++i, currentGain += gainStep){
        buffer[i] = std::clamp(buffer[i] * currentGain, -1.0f, 1.0f);
    }

    return true;
}