#pragma once

#include <algorithm>

#include "effect.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp {

/**
 * @brief Fade direction options.
 * 
 * - In  : Gradually increase gain from low to high.
 * - Out : Gradually decrease gain from high to low.
 */
enum FadeMode {
    In,  ///< Fade in
    Out  ///< Fade out
};

/**
 * @brief Parameters for the Fade effect.
 * 
 * This class stores configuration for applying a fade to an audio buffer:
 * - Start and end positions of the fade (in samples)
 * - Gain values at the start and end
 * - Fade direction (in/out)
 * 
 * Instances are created via the `create()` factory function to ensure
 * proper initialization and validation.
 */
class FadeParams : public EffectParams {
    float mHighGain;   ///< Gain at the louder end of the fade
    float mLowGain;    ///< Gain at the quieter end of the fade
    FadeMode mMode;    ///< Fade direction

    struct PrivateTag {}; ///< Tag to restrict direct construction
public:
    /**
     * @brief Factory method to create a FadeParams instance.
     * 
     * Validates and constructs parameters for a fade effect. 
     * - `highGain` must be greater than `lowGain`.
     * - Gains are clamped to the range [0.0f, 2.0f].
     * 
     * @param start     Sample position where the fade starts.
     * @param end       Sample position where the fade ends (inclusive).
     * @param highGain  Gain at the louder phase of the fade (clamped to [0.0f, 2.0f]).
     * @param lowGain   Gain at the quieter phase of the fade (clamped to [0.0f, 2.0f]).
     * @param mode      Fade direction (FadeMode::In or FadeMode::Out).
     * @param handler   Error handler for parameter validation failures.
     * 
     * @return Shared pointer to a valid FadeParams instance, or nullptr if parameters are invalid.
     */
    static std::shared_ptr<FadeParams> create(sample_pos& start, sample_pos& end,
        float& highGain, float& lowGain, FadeMode& mode, AJ::error::IErrorHandler &handler);

    /// @return Gain at the loud end of the fade.
    float highGain() { return mHighGain; }

    /// @return Gain at the quiet end of the fade.
    float lowGain() { return mLowGain; }

    /// @return Fade direction.
    FadeMode mode() { return mMode; }

    /**
     * @brief Sets the high and low gain values for the fade effect.
     * 
     * Validates the gain values and updates the internal parameters.
     * - `high` must be greater than `low`.
     * - Gains are clamped to the range [0.0f, 2.0f].
     * 
     * @param high     Gain at the louder phase of the fade.
     * @param low      Gain at the quieter phase of the fade.
     * @param handler  Error handler to report invalid parameters.
     * 
     * @return `true` if gains were successfully set, `false` if validation failed.
     */
    bool setGains(float high, float low, AJ::error::IErrorHandler &handler){
        if(low > high){
            const std::string message = "invalid gain parameters for fade effect.\n";
            handler.onError(error::Error::InvalidEffectParameters, message);

            return false;
        } 

        mHighGain = std::clamp(high, 0.0f, 2.0f);
        mLowGain = std::clamp(low, 0.0f, 2.0f);;

        return true;
    }

    /**
     * @brief Private constructor, accessible only via create().
     */
    FadeParams(PrivateTag) {
        mHighGain = 1.0f;
        mLowGain = 0.0f;
        mStart = 0;
        mEnd = 0;
        mMode = FadeMode::In;
    }
};

/**
 * @brief Fade effect processor.
 * 
 * Applies a linear gain ramp (fade in or fade out) over a specified range
 * of audio samples. The fade can either:
 *  - Increase gain from a low starting value to a higher value (fade in).
 *  - Decrease gain from a high starting value to a lower value (fade out).
 * 
 * This class supports:
 *  - A scalar ("naive") fade implementation for portability.
 *  - An AVX-optimized version for faster processing on CPUs with AVX support.
 * 
 * @details
 * The gain ramp is defined by:
 *   - `mStart` / `mEnd` sample positions (inclusive).
 *   - `mLowGain` and `mHighGain` values.
 *   - Fade mode (`FadeMode::In` or `FadeMode::Out`).
 * 
 * Gains are applied per-sample, with clamping to the range [-1.0f, 1.0f] to 
 * prevent distortion.
 */
class Fade : public AJ::dsp::Effect {
    std::shared_ptr<FadeParams> mParams; ///< Parameters controlling fade behavior.

    /**
     * @brief Scalar (non-SIMD) fade processing.
     * 
     * Processes samples in a straightforward loop, updating the gain per sample.
     * Intended as a fallback when SIMD acceleration is unavailable.
     * 
     * @param buffer   Audio buffer to process.
     * @param handler  Error handler for reporting parameter or processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool fadeNaive(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief AVX-optimized fade processing.
     * 
     * Uses 256-bit AVX vector operations to process 8 samples at a time, 
     * 
     * @param buffer   Audio buffer to process.
     * @param handler  Error handler for reporting parameter or processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool fadeAVX(Float &buffer, AJ::error::IErrorHandler &handler);

public:
    /**
     * @brief Default constructor.
     * 
     * Creates an uninitialized fade effect with no parameters set.
     * You must call `setParams()` before processing.
     */
    Fade() {
        mParams = nullptr;
    }

    /**
     * @brief Constructor with parameters.
     * 
     * Initializes the fade effect with given parameters pointer.
     * 
     * @param params   Shared pointer to fade parameters.
     * @param handler  Error handler for parameter validation.
     */
    Fade(std::shared_ptr<FadeParams> params, AJ::error::IErrorHandler &handler) {
        setParams(params, handler);
    }

    /**
     * @brief Apply the fade to an audio buffer.
     * 
     * Automatically selects between AVX-optimized and scalar fade 
     * implementations depending on CPU capabilities.
     * 
     * @param buffer   Audio samples to process (modified in place).
     * @param handler  Error handler for reporting processing issues.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Assigns fade parameters to the effect.
     * 
     * Verifies that the passed `EffectParams` pointer is of type `FadeParams`.  
     * If type mismatch occurs, an error is reported via the handler and 
     * parameters are not updated.
     * 
     * If the provided parameters ptr is of type FadeParams, 
     * they are assigned to `mParams`.
     * 
     * @param params   Shared pointer to `FadeParams` object.
     * @param handler  Error handler for reporting validation failures.
     * 
     * @return true if parameters were successfully assigned, false otherwise.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;
};

}; // namespace AJ::dsp
