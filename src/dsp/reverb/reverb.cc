#include <iostream>
#include "core/types.h"

#include "dsp/reverb/reverb.h"
#include "core/error_handler.h"
#include "core/effect_params.h"

std::shared_ptr<AJ::dsp::reverb::ReverbParams> AJ::dsp::reverb::ReverbParams::create(Params &params,
    AJ::error::IErrorHandler &handler){
    std::shared_ptr<ReverbParams> reverbParams = std::make_shared<ReverbParams>(PrivateTag{});
        
    if(params.mStart > params.mEnd || params.mStart < 0) {
        const std::string message = "invalid range indexes parameters for reverb effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    reverbParams->setStart(params.mStart);
    reverbParams->setEnd(params.mEnd);
    reverbParams->setDelayMS(params.mDelayMS);
    reverbParams->setWetMix(params.mWetMix);
    reverbParams->setDryMix(params.mDryMix);
    reverbParams->setSamplerate(params.mSamplerate);
    reverbParams->setGain(params.mGain);

    return reverbParams;
}

bool AJ::dsp::reverb::Reverb::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) {
    std::shared_ptr<ReverbParams> reverbParams = std::dynamic_pointer_cast<ReverbParams>(params);
    // if reverbParams is nullptr that means it's not a shared_ptr of ReverbParams
    if(!reverbParams){
        const std::string message = "Effect parameters must be of type ReverbParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    for (auto &all_pass : mAllPassFilters) {
        all_pass = std::make_unique<AllPassFilter>(reverbParams->Samplerate());
    }

    mParams = reverbParams; 
    return true;
}

bool AJ::dsp::reverb::Reverb::checkValidIndxes(Float &buffer, AJ::error::IErrorHandler &handler){
    if (buffer.empty()) {
        const std::string message = "empty buffer for reverb effect.\n";
        handler.onError(error::Error::EmptyAudioBuffer, message);
        return false;
    }

    if (mParams->Start() < 0 || mParams->End() < mParams->Start() || mParams->End() >= buffer.size()) {
        const std::string message = "invalid start/End indexes for reverb effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    // calculate total required delay in samples
    float total_delay_ms = mParams->DelayMS();  // base delay
    
    // Add maximum comb filter delay
    total_delay_ms += std::max({
        std::abs(COMB_FILTER_1_DELAY),
        std::abs(COMB_FILTER_2_DELAY),
        std::abs(COMB_FILTER_3_DELAY)
    });
    
    // Add all-pass filter delay
    total_delay_ms += 89.27f;  // all-pass filter delay
    
    // Convert to samples
    sample_pos required_samples = static_cast<sample_pos>((total_delay_ms / 1000.0f) * mParams->Samplerate());
    
    // We need enough samples after our current position for the delay
    if (buffer.size() < (required_samples * 2)) {  // Double the requirement for safety
        const std::string message = "Buffer too small for reverb. Need at least " 
                  + std::to_string(required_samples * 2) + " samples, but have " 
                  + std::to_string(buffer.size()) + " samples.\n";

        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }
    
    // Make sure we have enough samples remaining after 'mParams->Start()'
    if ((buffer.size() - mParams->Start()) < required_samples) {
        const std::string message = "buffer too small for reverb delay.\n";

        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    return true;
}

bool AJ::dsp::reverb::Reverb::process(Float &buffer, AJ::error::IErrorHandler &handler){
    
    if(!checkValidIndxes(buffer, handler)) return false;

    // set the comb filters.
    sample_c size = mParams->End() - mParams->Start() + 1;
    mCombFilters[0]->setDelay(mParams->DelayMS(), mParams->Samplerate(), size, handler);
    mCombFilters[0]->setGain(mParams->Gain());

    mCombFilters[1]->setDelay(mParams->DelayMS() + COMB_FILTER_1_DELAY, mParams->Samplerate(), size, handler);
    mCombFilters[1]->setGain(mParams->Gain());

    mCombFilters[2]->setDelay(mParams->DelayMS() + COMB_FILTER_2_DELAY, mParams->Samplerate(), size, handler);
    mCombFilters[2]->setGain(mParams->Gain());

    mCombFilters[3]->setDelay(mParams->DelayMS() + COMB_FILTER_3_DELAY, mParams->Samplerate(), size, handler);
    mCombFilters[3]->setGain(mParams->Gain());


    Float output(size, 0.0f);
    Float comb1_out(size, 0.0f);
    Float comb2_out(size, 0.0f);
    Float comb3_out(size, 0.0f);
    Float comb4_out(size, 0.0f);


    for(sample_c i = mParams->Start(); i <= mParams->End(); ++i){
        sample_t s1 = mCombFilters[0]->process(buffer, comb1_out, i, mParams->Start());
        sample_t s2 = mCombFilters[1]->process(buffer, comb2_out, i, mParams->Start());
        sample_t s3 = mCombFilters[2]->process(buffer, comb3_out, i, mParams->Start());
        sample_t s4 = mCombFilters[3]->process(buffer, comb4_out, i, mParams->Start());

        output[i - mParams->Start()] = (s1 + s2 + s3 + s4) / 4; // normalization.
    }

    // sequential processing for all pass filters 
    Float all_pass_out = mAllPassFilters[0]->process(output);
    all_pass_out = mAllPassFilters[1]->process(all_pass_out);

    // wet and dry mixing and copying into main buffer
    size_t j = mParams->Start();
    sample_t sample;
    for(size_t i = 0; i < size; ++i, ++j){
        sample = mParams->WetMix() * all_pass_out[i] + mParams->DryMix() * buffer[j];
        buffer[j] = std::clamp(sample, -1.0f, 1.0f); 
    }

    return true;
}