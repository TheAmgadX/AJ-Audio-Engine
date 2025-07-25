#include <iostream>
#include "core/types.h"

#include "dsp/reverb/reverb.h"


bool AJ::dsp::reverb::Reverb::checkValidIndxes(Float &buffer, sample_pos start, sample_pos end){
    if (buffer.empty()) {
        std::cerr << "empty buffer for reverb effect.\n";
        return false;
    }

    if (start < 0 || end < start || end >= buffer.size()) {
        std::cerr << "invalid start/end indexes for reverb effect.\n";
        return false;
    }

    // calculate total required delay in samples
    float total_delay_ms = mDelayMS;  // base delay
    
    // Add maximum comb filter delay
    total_delay_ms += std::max({
        std::abs(COMB_FILTER_1_DELAY),
        std::abs(COMB_FILTER_2_DELAY),
        std::abs(COMB_FILTER_3_DELAY)
    });
    
    // Add all-pass filter delay
    total_delay_ms += 89.27f;  // all-pass filter delay
    
    // Convert to samples
    sample_pos required_samples = static_cast<sample_pos>((total_delay_ms / 1000.0f) * mSamplerate);
    
    // We need enough samples after our current position for the delay
    if (buffer.size() < (required_samples * 2)) {  // Double the requirement for safety
        std::cerr << "Buffer too small for reverb. Need at least " 
                  << required_samples * 2 << " samples, but have " 
                  << buffer.size() << " samples.\n";
        return false;
    }
    
    // Make sure we have enough samples remaining after 'start'
    if ((buffer.size() - start) < required_samples) {
        std::cerr << "buffer too small for reverb delay.\n";
        return false;
    }

    return true;
}

void AJ::dsp::reverb::Reverb::process(Float &buffer, sample_pos start, sample_pos end){
    
    if(!checkValidIndxes(buffer, start, end)) return;

    // set the comb filters.
    sample_c size = end - start + 1;
    mCombFilters[0]->setDelay(mDelayMS, mSamplerate, size);
    mCombFilters[0]->setGain(mGain);

    mCombFilters[1]->setDelay(mDelayMS + COMB_FILTER_1_DELAY, mSamplerate, size);
    mCombFilters[1]->setGain(mGain);

    mCombFilters[2]->setDelay(mDelayMS + COMB_FILTER_2_DELAY, mSamplerate, size);
    mCombFilters[2]->setGain(mGain);

    mCombFilters[3]->setDelay(mDelayMS + COMB_FILTER_3_DELAY, mSamplerate, size);
    mCombFilters[3]->setGain(mGain);


    Float output(size, 0.0f);
    Float comb1_out(size, 0.0f);
    Float comb2_out(size, 0.0f);
    Float comb3_out(size, 0.0f);
    Float comb4_out(size, 0.0f);


    for(sample_c i = start; i <= end; ++i){
        sample_t s1 = mCombFilters[0]->process(buffer, comb1_out, i, start);
        sample_t s2 = mCombFilters[1]->process(buffer, comb2_out, i, start);
        sample_t s3 = mCombFilters[2]->process(buffer, comb3_out, i, start);
        sample_t s4 = mCombFilters[3]->process(buffer, comb4_out, i, start);

        output[i - start] = (s1 + s2 + s3 + s4) / 4; // normalization.
    }

    // sequential processing for all pass filters 
    Float all_pass_out = mAllPassFilters[0]->process(output);
    all_pass_out = mAllPassFilters[1]->process(all_pass_out);

    // wet and dry mixing and copying into main buffer
    size_t j = start;
    sample_t sample;
    for(size_t i = 0; i < size; ++i, ++j){
        sample = mWetMix * all_pass_out[i] + mDryMix * buffer[j];
        buffer[j] = std::clamp(sample, -1.0f, 1.0f); 
    }
}