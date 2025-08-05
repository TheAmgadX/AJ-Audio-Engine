#pragma once
#include <iostream>
#include <algorithm>

#include "core/types.h"

namespace AJ::dsp::reverb {

/**
 * @class AllPassFilter
 * @brief Implements a simple all-pass filter used in audio reverb processing.
 *
 * This filter delays the input signal and feeds it back with a gain to create phase shifting
 * without affecting the amplitude response. It is used in reverb effect.
 */
class AllPassFilter {

private:
    float mDelayMS;       ///< Delay time in milliseconds
    sample_c mDelay;      ///< Delay in samples
    int mSamplerate;      ///< Audio sample rate (Hz)
    float mGain;          ///< Feedback gain

public:

    /**
     * @brief Default constructor.
     * 
     * Initializes with:
     * - delay = 89.27 ms
     * - gain = 0.131
     * - sample rate = 44100 Hz
     */
    AllPassFilter(){
        mDelayMS = 89.27f;
        mGain = 0.131f;
        mSamplerate = 44100;
        mDelay = mDelayMS * (float(mSamplerate) / 1000.0f);
    }

    /**
     * @brief Constructs the filter with a given sample rate.
     * 
     * @param samplerate Audio sample rate (Hz)
     */
    AllPassFilter(int samplerate){
        mDelayMS = 89.27f; 
        mGain = 0.131f;
        mSamplerate = samplerate;
        mDelay = mDelayMS * (float(mSamplerate) / 1000.0f);
    }

    /**
     * @brief Sets the number of delayed samples based on the given delay time in milliseconds.
     *
     * @param delayMS Delay in milliseconds.
     * @param samplerate Current audio sample rate (used to convert ms to samples).
     */
    void setDelay(float delayMS, const sample_c samplerate){
        mDelay = (float(samplerate) / 1000) * delayMS;
    }
    
    /**
     * @brief Sets the feedback gain, clamped between REVERB_GAIN_MIN and REVERB_GAIN_MAX.
     *
     * @param gain Feedback gain value.
     * @see include/core/constants.h
     */
    void setGain(float gain){
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    /**
     * @brief Sets the sample rate and recalculates the internal delay.
     *
     * @param samplerate Audio sample rate (Hz), clamped between 8100 and 44100.
     */
    void setSamplerate(int samplerate){
        mSamplerate = std::clamp(samplerate, 8100, 44100);
        setDelay(mDelayMS, samplerate);
    }

    /**
     * @brief Applies the all-pass filter to the input buffer.
     * 
     * @param input Audio buffer to be processed.
     * @return Processed audio buffer.
     */
    Float process(Float &input);
};

};
