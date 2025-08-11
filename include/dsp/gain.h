#pragma once
#include <iostream>
#include <memory>

#include "effect.h"
#include "core/types.h"
#include "core/error_handler.h"

namespace AJ::dsp {

/**
 * @brief Parameters for the Gain effect.
 *
 * Holds the gain value and the range (start and end positions) where
 * the effect should be applied.
 */
class GainParams : public EffectParams {
public:
    /**
     * @brief The gain multiplier to apply to the audio samples.
     */
    gain_t mGain;

    ~GainParams() override = default;

    /**
     * @brief Default constructor, sets start and end positions to -1.
     */
    GainParams() {
        mStart = -1;
        mEnd = -1;
    }

    /**
     * @brief Constructor with explicit range values.
     * 
     * @param start The starting sample position.
     * @param end The ending sample position.
     */
    GainParams(sample_pos start, sample_pos end) {
        mStart = start;
        mEnd = end;
    }
};

/**
 * @brief Gain effect class that scales audio samples by a gain factor.
 */
class Gain : public AJ::dsp::Effect {

private:
    /**
     * @brief Parameters for the Gain effect.
     */
    std::shared_ptr<GainParams> mParams;

    /**
     * @brief Apply gain to a single sample (naive implementation).
     * 
     * @param sample Reference to the sample to be modified.
     */
    void calculate_gain_sample(sample_t &sample);

    /**
     * @brief Naive gain implementation for the entire buffer.
     * 
     * @param buffer Audio buffer to process.
     * @param handler Error handler for reporting issues.
     * 
     * @return true on success, false on failure.
     */
    bool gainNaive(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief AVX-accelerated gain implementation (optional optimization).
     * 
     * @param buffer Audio buffer to process.
     * @param handler Error handler for reporting issues.
     * 
     * @return true on success, false on failure.
     */
    bool gainAVX(Float &buffer, AJ::error::IErrorHandler &handler);

public:
    /**
     * @brief Set the gain value after validating it.
     * Can also be used to mute audio by setting the gain to 0.0f.
     * 
     * @param gain The new gain value to apply. Must be in [0.0, 5.0].
     * @param handler Error handler to report invalid gain.
     * 
     * @return true if the gain is valid and applied, false otherwise.
     */
    bool setGain(gain_t gain, AJ::error::IErrorHandler &handler) {
        if (gain < 0.0 || gain > 5.0) {
            const std::string message = "Invalid gain: " + std::to_string(gain) 
                + " Gain must be in range of [0.0, 5.0]\n";

            handler.onError(error::Error::InvalidEffectParameters, message);
            return false;
        }
        mParams->mGain = gain;
        return true;
    }

    /**
     * @brief Default constructor. Initializes gain to 1.
     */
    Gain() {
        mParams = std::make_shared<GainParams>();
        mParams->mGain = 1;
    }

    /**
     * @brief Get the current gain value.
     * 
     * @return Gain value.
     */
    gain_t gain() {
        return mParams->mGain;
    }

    /**
     * @brief Set effect parameters (casted to GainParams internally).
     * 
     * @param params Pointer to EffectParams (should be GainParams).
     * @param handler Error handler for type mismatch or invalid values.
     * 
     * @return true if parameters are valid and applied, false otherwise.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Set the start and end sample positions for the effect range.
     * 
     * @param start Start position (inclusive).
     * @param end End position (inclusive).
     */
    void setRange(sample_pos start, sample_pos end) {
        if (start <= end) {
            mParams->mStart = start;
            mParams->mEnd = end;
        }
    }

    /**
     * @brief Process the audio buffer using the gain effect.
     * 
     * @param buffer Audio buffer to apply gain on.
     * @param handler Error handler for any runtime issues.
     * 
     * @return true on success, false on failure.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

};

};
