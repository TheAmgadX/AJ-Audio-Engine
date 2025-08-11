#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>

#include "dsp/normalization.h"
#include "dsp/gain.h"
#include "core/types.h"
#include "core/error_handler.h"

std::shared_ptr<AJ::dsp::NormalizationParams> AJ::dsp::NormalizationParams::create(sample_pos& start,
    sample_pos& end, AJ::error::IErrorHandler &handler, float target, NormalizationMode mode){

    std::shared_ptr<NormalizationParams> params = std::make_shared<NormalizationParams>(PrivateTag{});
        
    if(start > end || start < 0) {
        const std::string message = "invalid range indexes parameters for normalization effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    params->setTarget(target);
    params->mStart = start;
    params->mEnd = end;
    params->mMode = mode;

    return params;
}

bool AJ::dsp::Normalization::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
    std::shared_ptr<NormalizationParams> normalizationParams = std::dynamic_pointer_cast<NormalizationParams>(params);
    // if normalizationParams is nullptr that means it's not a shared_ptr of normalizationParams
    if(!normalizationParams){
        const std::string message = "Effect parameters must be of type NormalizationParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = normalizationParams; 
    return true;
}

bool AJ::dsp::Normalization::process(Float &buffer, AJ::error::IErrorHandler &handler){
    if(mParams->Mode() == NormalizationMode::Peak)
        return normalizationPeak(buffer, handler);
    
    return normalizationRMS(buffer, handler);
}

bool AJ::dsp::Normalization::normalizationPeak(Float &buffer, AJ::error::IErrorHandler &handler){
    /**
     * Peak normalization adjusts audio so that the loudest sample
     * reaches a user-defined target peak level.
     *
     * Steps:
     * 1. Find the maximum absolute sample value in the buffer:
     *        max_sample = max(max_sample, abs(current_sample))
     *
     * 2. Calculate the gain needed to make this peak match the target:
     *        gain = target / max_sample
     *
     * 3. Multiply all samples by the gain:
     *        buffer[i] *= gain;
     *
     * Notes:
     * - If the target is lower than the current peak, the gain will be < 1.0,
     *   which reduces the volume instead of increasing it. This is perfectly
     *   valid and is used to attenuate audio.
     */


    // find the max:
    float max_sample = -1.0f;

    for(size_t i = mParams->mStart; i <= mParams->mEnd; ++i)
        max_sample = std::max(max_sample, std::abs(buffer[i]));

    
    // calc gain:
    float gain_factor = mParams->Target() / max_sample;
    mParams->setGain(gain_factor);

    // use special gain functionality that doesn't need clamping


    return gain(buffer, handler);
}

// TODO: change the code if abstract factory used for gain
bool AJ::dsp::Normalization::normalizationRMS(Float &buffer, AJ::error::IErrorHandler &handler){
    /**
     * RMS normalization process (current implementation):
     *
     * 1. Compute the RMS (Root Mean Square) of all samples:
     *      sum_sq = sum(sample^2) for each sample
     *      mean_sq = sum_sq / num_samples
     *      RMS = sqrt(mean_sq)
     *
     * 2. Compute peak amplitude across the same range:
     *      max_sample = max(abs(sample))
     *
     * 3. Calculate desired RMS gain:
     *      gain_rms = target_linear / RMS
     *
     * 4. Calculate peak-safe gain (to prevent clipping):
     *      gain_peak = target_linear / max_sample
     *
     * 5. Use the smaller of the two gains:
     *      gain = min(gain_rms, gain_peak)
     *
     * 6. Apply the gain to all samples:
     *      sample *= gain
     *
     * NOTE: This approach effectively performs RMS normalization
     *       but clamps it using peak normalization to avoid clipping.
     *
     * TODO: Replace the simple gain clamp with a proper peak limiter
     */


    double sum_sq = 0.0f;
    float max_sample = -1.0f;
    for(size_t i = mParams->mStart; i <= mParams->mEnd; ++i){
        sum_sq += std::pow(buffer[i], 2);
        max_sample = std::max(max_sample, std::abs(buffer[i]));
    }

    float mean_sq = sum_sq / (mParams->mEnd - mParams->mStart + 1);

    float RMS = std::sqrt(mean_sq);

    float gain = mParams->Target() / RMS;

    // peak limiting.
    gain = std::min(gain, mParams->Target() / max_sample);

    mParams->setGain(gain);

    // start applying gain
    std::shared_ptr<AJ::dsp::GainParams> gainParams = 
        std::make_shared<AJ::dsp::GainParams>(mParams->mStart, mParams->mEnd); 
    
    gainParams->mGain = mParams->Gain();

    AJ::dsp::Gain Gain;

    if(!Gain.setParams(gainParams, handler)){
        return false;
    }

    return Gain.process(buffer, handler);
}