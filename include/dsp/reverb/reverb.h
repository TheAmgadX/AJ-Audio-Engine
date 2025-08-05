#pragma once

#include <iostream>
#include <memory>
#include <algorithm>

#include "dsp/effect.h"
#include "core/types.h"
#include "dsp/reverb/all_pass_filter.h"
#include "dsp/reverb/comb_filter.h"

namespace AJ::dsp::reverb {

/**
 * @brief Parameters used for configuring the Reverb effect.
 */
class ReverbParams : public EffectParams {
public:
    float mDelayMS;       /**< Delay time in milliseconds. */
    float mWetMix;        /**< Volume of the wet (processed) signal. */
    float mDryMix;        /**< Volume of the dry (original) signal. */
    int mSamplerate;      /**< Sampling rate of the audio in Hz. */
    float mGain;          /**< Feedback gain for reverb intensity. */

    ~ReverbParams() override = default;

    /**
     * @brief Default constructor with unset range.
     */
    ReverbParams() {
        mStart = -1;
        mEnd = -1;
    }

    /**
     * @brief Constructor with start and end sample positions.
     * 
     * @param start Start sample index.
     * @param end End sample index.
     */
    ReverbParams(sample_pos start, sample_pos end) {
        mStart = start;
        mEnd = end;
    }
};

using CombFilters = std::array<std::unique_ptr<CombFilter>, kCombFilters>;
using AllPassFilters = std::array<std::unique_ptr<AllPassFilter>, kAllPassFilters>;

/**
 * @brief Reverb effect implementation.
 * 
 * Adds reverberation (echo-like reflections) to audio using a combination of comb and all-pass filters.
 * Supports adjustable parameters such as delay, gain, dry/wet mix.
 * 
 * TODO: Add support for different reverb presets (e.g., room, hall, cathedral).
 * 
 * Example presets:
 * | Reverb Type | Delay (ms) | Wet Mix | Dry Mix | Gain |
 * |-------------|------------|---------|---------|------|
 * | Room        | 25.0       | 0.3     | 0.7     | 0.4  |
 * | Hall        | 70.0       | 0.5     | 0.5     | 0.7  |
 * | Cathedral   | 110.0      | 0.8     | 0.2     | 0.9  |
 */

class Reverb : public AJ::dsp::Effect {

private:
    std::shared_ptr<ReverbParams> mParams;     /**< Parameters for the reverb effect. */
    CombFilters mCombFilters;                  /**< Array of comb filters used to simulate echo buildup. */
    AllPassFilters mAllPassFilters;            /**< Array of all-pass filters used to smooth echo tails. */

    /**
     * @brief Validates the processing range.
     */
    bool checkValidIndxes(Float &buffer, AJ::error::IErrorHandler &handler);

public:
    /**
     * @brief Default constructor initializing filters and default parameters.
     */
    Reverb() {
        mParams = std::make_shared<ReverbParams>();

        mParams->mGain = REVERB_GAIN;
        mParams->mDelayMS = REVERB_DELAY;
        mParams->mDryMix = REVERB_DRY_MIX;
        mParams->mWetMix = REVERB_WET_MIX;

        for (auto &comb : mCombFilters) {
            comb = std::make_unique<CombFilter>();
        }

        for (auto &all_pass : mAllPassFilters) {
            all_pass = std::make_unique<AllPassFilter>(mParams->mSamplerate);
        }
    }

    /**
     * @brief Constructor with sample rate.
     * 
     * @param samplerate Sample rate of the audio in Hz.
     */
    Reverb(int samplerate) {
        mParams = std::make_shared<ReverbParams>();
        mParams->mSamplerate = samplerate;
        mParams->mGain = REVERB_GAIN;
        mParams->mDelayMS = REVERB_DELAY;
        mParams->mDryMix = REVERB_DRY_MIX;
        mParams->mWetMix = REVERB_WET_MIX;

        for (auto &comb : mCombFilters) {
            comb = std::make_unique<CombFilter>();
        }

        for (auto &all_pass : mAllPassFilters) {
            all_pass = std::make_unique<AllPassFilter>(mParams->mSamplerate);
        }
    }

    /**
     * @brief Sets the reverb delay time in milliseconds.
     */
    void setDelay(float delayMS) {
        mParams->mDelayMS = std::clamp(delayMS,
            static_cast<float>(REVERB_DELAY_MIN),
            static_cast<float>(REVERB_DELAY_MAX));
    }

    /**
     * @brief Sets the dry signal mix (original audio).
     */
    void setDryMix(float mix) {
        mParams->mDryMix = std::clamp(mix,
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX));
    }

    /**
     * @brief Sets the wet signal mix (processed audio).
     */
    void setWetMix(float mix) {
        mParams->mWetMix = std::clamp(mix,
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX));
    }

    /**
     * @brief Sets the reverb gain (feedback strength).
     */
    void setGain(float gain) {
        mParams->mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    /**
     * @brief Sets the audio sample rate.
     */
    void setSamplerate(int samplerate) {
        mParams->mSamplerate = samplerate;
    }

    /**
     * @brief Sets effect parameters using a shared EffectParams pointer.
     * 
     * @param params A polymorphic pointer to effect parameters.
     * @param handler Error handler for reporting issues.
     * 
     * @return true if parameters were valid and accepted.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Defines the range of samples to process.
     * 
     * @param start Start sample index (inclusive).
     * @param end End sample index (inclusive).
     */
    void setRange(sample_pos start, sample_pos end) {
        if (start <= end) {
            mParams->mStart = start;
            mParams->mEnd = end;
        }
    }

    /**
     * @brief Processes the audio buffer with reverb.
     * 
     * @param buffer Audio buffer to modify.
     * @param handler Error handler for reporting runtime issues.
     * 
     * @return true if processing succeeded.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;
};

};
