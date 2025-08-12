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
 * @brief Simple struct holding raw reverb configuration values.
 *        must set all values for initializing reverb effect.
 */
struct Params {
    float mDelayMS;       /**< Delay time in milliseconds. */
    float mWetMix;        /**< Volume of the wet (processed) signal. */
    float mDryMix;        /**< Volume of the dry (original) signal. */
    int mSamplerate;      /**< Sampling rate of the audio in Hz. */
    float mGain;          /**< Feedback gain for reverb intensity. */

    sample_c mStart;      /**< Sample position where reverb starts. */
    sample_c mEnd;        /**< Sample position where reverb ends (inclusive). */
};

/**
 * @brief Parameters used for configuring the Reverb effect.
 */
class ReverbParams : public EffectParams {
    struct PrivateTag {}; /**< Internal tag for controlled construction. */

    float mDelayMS;       /**< Delay time in milliseconds. */
    float mWetMix;        /**< Volume of the wet (processed) signal. */
    float mDryMix;        /**< Volume of the dry (original) signal. */
    int mSamplerate;      /**< Sampling rate of the audio in Hz. */
    float mGain;          /**< Feedback gain for reverb intensity. */

public:

    /**
     * @brief Factory method to create a ReverbParams instance.
     * 
     * Validates and constructs parameters for a Reverb effect. 
     * This is the only intended way to create an instance of ReverbParams,
     * because although the constructor is technically public, it requires a
     * special PrivateTag parameter that cannot be accessed outside this class.
     * This pattern enforces controlled initialization while keeping the API clean.
     * 
     * @param params    Struct containing all parameters.
     * @param handler   Error handler for parameter validation failures.
     * @return Shared pointer to a valid ReverbParams instance, or nullptr if parameters are invalid.
     */
    static std::shared_ptr<ReverbParams> create(Params &params, AJ::error::IErrorHandler &handler);

    /**
     * @brief Sets the delay time in milliseconds.
     * @param delayMS New delay time.
     */
    void setDelayMS(float delayMS) {
        mDelayMS = std::clamp(delayMS,
            static_cast<float>(REVERB_DELAY_MIN),
            static_cast<float>(REVERB_DELAY_MAX));
    }

    /**
     * @brief Sets the wet mix (processed signal level).
     * @param wetMix New wet mix value.
     */
    void setWetMix(float wetMix) {
        mWetMix = std::clamp(wetMix,
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX));
    }

    /**
     * @brief Sets the dry mix (original signal level).
     * @param dryMix New dry mix value.
     */
    void setDryMix(float dryMix) {
        mDryMix = std::clamp(dryMix,
            static_cast<float>(REVERB_MIX_MIN),
            static_cast<float>(REVERB_MIX_MAX));
    }

    /**
     * @brief Sets the audio sample rate.
     * init the all pass filters.
     * 
     * @param samplerate New sample rate in Hz.
     */
    void setSamplerate(int samplerate) {
        mSamplerate = samplerate;
    }

    /**
     * @brief Sets the reverb feedback gain.
     * @param gain New gain value.
     */
    void setGain(float gain) {
        mGain = std::clamp(gain, REVERB_GAIN_MIN, REVERB_GAIN_MAX);
    }

    /**
     * @brief Returns the delay time in milliseconds.
     */
    float DelayMS() const { return mDelayMS; }

    /**
     * @brief Returns the wet mix value.
     */
    float WetMix() const { return mWetMix; }

    /**
     * @brief Returns the dry mix value.
     */
    float DryMix() const { return mDryMix; }

    /**
     * @brief Returns the feedback gain.
     */
    float Gain() const { return mGain; }

    /**
     * @brief Returns the sample rate in Hz.
     */
    int Samplerate() const { return mSamplerate; }

    /**
     * @brief Destructor.
     */
    ~ReverbParams() override = default;

    /**
     * @brief Default constructor with unset start/end range.
     */
    ReverbParams(PrivateTag) {
        setStart(-1);
        setEnd(-1);
    }
};

/// Type alias for the comb filter array used in the reverb.
using CombFilters = std::array<std::unique_ptr<CombFilter>, kCombFilters>;

/// Type alias for the all-pass filter array used in the reverb.
using AllPassFilters = std::array<std::unique_ptr<AllPassFilter>, kAllPassFilters>;

/**
 * @brief Reverb effect implementation.
 * 
 * Adds reverberation (echo-like reflections) to audio using a combination of comb and all-pass filters.
 * Supports adjustable parameters such as delay, gain, dry/wet mix.
 */
class Reverb : public AJ::dsp::Effect {

private:
    std::shared_ptr<ReverbParams> mParams;     /**< Parameters for the reverb effect. */
    CombFilters mCombFilters;                  /**< Array of comb filters used to simulate echo buildup. */
    AllPassFilters mAllPassFilters;            /**< Array of all-pass filters used to smooth echo tails. */

    /**
     * @brief Validates the processing range indexes.
     * 
     * @param buffer Audio buffer to check.
     * @param handler Error handler for reporting invalid indexes.
     * 
     * @return true if indexes are valid.
     */
    bool checkValidIndxes(Float &buffer, AJ::error::IErrorHandler &handler);

public:
    /**
     * @brief Default constructor initializing filters and default parameters.
     */
    Reverb(){
        mParams = nullptr;

        for (auto &comb : mCombFilters) {
            comb = std::make_unique<CombFilter>();
        }
    }

    /**
     * @brief Sets the reverb delay time in milliseconds.
     * @param val New delay time in ms.
     */
    void setDelayMS(float val) { mParams->setDelayMS(val); }

    /**
     * @brief Sets the dry signal mix (original audio).
     * @param val New dry mix value.
     */
    void setDryMix(float val) { mParams->setDryMix(val); }

    /**
     * @brief Sets the wet signal mix (processed audio).
     * @param val New wet mix value.
     */
    void setWetMix(float val) { mParams->setWetMix(val); }

    /**
     * @brief Sets the reverb gain (feedback strength).
     * @param val New gain value.
     */
    void setGain(float val) { mParams->setGain(val); }

    /**
     * @brief Sets the audio sample rate.
     * @param val New sample rate in Hz.
     */
    void setSamplerate(int val) { 
        mParams->setSamplerate(val); 

        for (auto &all_pass : mAllPassFilters) {
            all_pass = std::make_unique<AllPassFilter>(mParams->Samplerate());
        }
    }

    /**
     * @brief Sets effect parameters using a shared EffectParams pointer.
     * must be called before process()
     * 
     * @param params A polymorphic pointer to effect parameters.
     * @param handler Error handler for reporting issues.
     * 
     * @return true if parameters were valid and accepted.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Defines the range of samples to process.
     * @param start Start sample index (inclusive).
     * @param end End sample index (inclusive).
     */
    void setRange(sample_pos start, sample_pos end) {
        if (start <= end) {
            mParams->setStart(start);
            mParams->setEnd(end);
        }
    }
    
    /**
     * @brief Gets the current reverb delay in ms.
     */
    float DelayMS() const { return mParams->DelayMS(); }

    /**
     * @brief Gets the current wet mix.
     */
    float WetMix() const { return mParams->WetMix(); }

    /**
     * @brief Gets the current dry mix.
     */
    float DryMix() const { return mParams->DryMix(); }

    /**
     * @brief Gets the current sample rate in Hz.
     */
    int Samplerate() const { return mParams->Samplerate(); }

    /**
     * @brief Gets the current reverb gain.
     */
    float Gain() const { return mParams->Gain(); }

    /**
     * @brief Gets the start sample index for processing.
     */
    sample_pos Start() const { return mParams->Start(); }

    /**
     * @brief Gets the end sample index for processing.
     */
    sample_pos End() const { return mParams->End(); }
    
    /**
     * @brief Processes the audio buffer with reverb.
     * @param buffer Audio buffer to modify.
     * @param handler Error handler for reporting runtime issues.
     * @return true if processing succeeded.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;
};

}; // namespace AJ::dsp::reverb
