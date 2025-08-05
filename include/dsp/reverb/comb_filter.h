#pragma once
#include <iostream>
#include <algorithm>

#include "core/types.h"
#include "core/error_handler.h"

namespace AJ::dsp::reverb {

/**
 * @brief A comb filter used in reverb processing.
 * 
 * This filter applies a delay-based feedback loop to simulate echo and coloration of the input signal.
 */
class CombFilter {
private:
    sample_c mDelay;  ///< Delay in samples.
    float mGain;      ///< Feedback gain factor.

public:
    /**
     * @brief Sets the delay of the comb filter.
     * 
     * @param delayMS Delay time in milliseconds.
     * @param samplerate Sampling rate in Hz.
     * @param size Size of the audio buffer.
     * @param handler Error handler for reporting runtime issues.
     * 
     * @return true if the delay was set successfully.
     * @return false if the delay is invalid or exceeds buffer size.
     * 
     * @see include/core/constants.h
     */
    bool setDelay(float delayMS, const int samplerate, const sample_c size,
        AJ::error::IErrorHandler &handler){

        if(delayMS <= 0) {
            mDelay = (REVERB_DELAY / 1000) * samplerate; // 78.9ms 
            return false;
        }

        mDelay = (delayMS / 1000) * samplerate;

        if(mDelay >= size){
            const std::string message = "invalid delay: delay is longer than the buffer size\n"; 
            handler.onError(AJ::error::Error::InvalidProcessingRange, message);

            return false;
        }

        return true;
    }

    /**
     * @brief Sets the feedback gain of the comb filter.
     * 
     * @param gain Gain value (clamped between REVERB_GAIN_MIN and REVERB_GAIN_MAX).
     * 
     * @see include/core/constants.h
     */
    void setGain(float gain){
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    /**
     * @brief Processes the audio signal with the comb filter effect.
     * 
     * This function assumes that all ranges and indexes have been validated beforehand.
     * No additional checks or error handlers are required when calling it.
     * 
     * @param input The input audio buffer.
     * @param output The output audio buffer.
     * @param idx Current sample index.
     * @param start Starting index of processing.
     * 
     * @return Processed sample value.
     */
    sample_t process(Float &input, Float &output, sample_pos idx, sample_pos start);
};

};
