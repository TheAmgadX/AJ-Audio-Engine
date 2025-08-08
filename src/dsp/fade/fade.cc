#include <iostream>
#include <algorithm>

#include "dsp/fade.h"
#include "core/types.h"
#include "core/error_handler.h"

std::shared_ptr<AJ::dsp::FadeParams> AJ::dsp::FadeParams::create(const sample_pos& start, const sample_pos& end,
    const float& highGain, const float& lowGain, const FadeMode& mode, AJ::error::IErrorHandler &handler){

    if(lowGain > highGain){
        const std::string message = "invalid gain parameters for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    } 

    std::shared_ptr<FadeParams> params = std::make_shared<FadeParams>(PrivateTag{});

    params->mLowGain = std::clamp(lowGain, 0.0f, 2.0f);
    params->mHighGain = std::clamp(highGain, 0.0f, 2.0f);
    params->mStart = start;
    params->mEnd = end;
    params->mMode = mode;

    return params;
}

bool AJ::dsp::Fade::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
    std::shared_ptr<FadeParams> fadeParams = std::dynamic_pointer_cast<FadeParams>(params);
    // if fadeInParams is nullptr that means it's not a shared_ptr of fadeInParams
    if(!fadeParams){
        const std::string message = "Effect parameters must be of type FadeParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = fadeParams; 
    return true;
}

bool AJ::dsp::Fade::process(Float &buffer, AJ::error::IErrorHandler &handler){
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return fadeAVX(buffer, handler);
    }
#endif

    return fadeNaive(buffer, handler);
}

bool AJ::dsp::Fade::fadeNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    /**
     * Fade equation:
     *   output[i] = input[i] * currentGain
     *
     * currentGain starts at lowGain if fade in and highGain if fade out
     * and increases/decreases by gainStep for each sample.
     *
     * gainDifference = highGain - lowGain
     * totalSamples   = mEnd - mStart
     * gainStep       = gainDifference / totalSamples
     * 
     * gainStep *= -1 if it's fade out.
     * 
     * currentGain = highGain if fade out
     * currentGain = lowGain if fade in
     */


    // check valid indexes range.
    if(mParams->mEnd < mParams->mStart || mParams->mStart < 0 || mParams->mEnd >= buffer.size()){
        const std::string message = "invalid indexes for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return false;
    }

    float currentGain;

    float gainDiff = mParams->highGain() - mParams->lowGain();
    sample_c totalSamples = mParams->mEnd - mParams->mStart + 1;
    double gainStep = gainDiff / totalSamples;

    if(mParams->mode() == FadeMode::In){
        currentGain = mParams->lowGain();
    } else {
        currentGain = mParams->highGain();
        gainStep *= -1;
    }


    for(size_t i = mParams->mStart; i <= mParams->mEnd; ++i, currentGain += gainStep){
        buffer[i] = std::clamp(buffer[i] * currentGain, -1.0f, 1.0f);
    }

    return true;
}