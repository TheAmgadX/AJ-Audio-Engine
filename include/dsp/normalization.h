#pragma once

#include <algorithm>
#include <memory>
#include <cmath>

#include "effect.h"
#include "core/effect_params.h"

namespace AJ::dsp {

/**
 * @brief Modes of normalization.
 *
 * - Peak: Scales audio so that the highest peak reaches the target amplitude.
 * - RMS:  Scales audio so that the RMS (Root Mean Square) value reaches the target amplitude.\
 * 
 * * NOTE: RMS normalization in this implementation is not fully tuned for
 *   practical use — it may produce excessive gain for low-level or sparse audio.
 */
enum NormalizationMode {
    Peak,  ///< Peak normalization mode.
    RMS,   ///< RMS normalization mode.
};

/**
 * @brief Parameters for the Normalization effect.
 *
 * Stores target amplitude, selected normalization mode,
 * and computed gain value. Provides clamped setters to ensure
 * parameters stay within valid ranges.
 */
class NormalizationParams : public EffectParams {
    float mTarget;               ///< Target amplitude (linear scale, [0.0f, 1.0f]).
    float mGain;                 ///< Computed gain factor to apply.
    NormalizationMode mMode;     ///< Selected normalization mode.

    struct PrivateTag {};        ///< Tag for private constructor to enforce factory creation.
public:
    /**
     * @brief Factory method to create a NormalizationParams instance.
     * 
     * Validates and constructs parameters for a Normalization effect. 
     * 
     * @param start     Sample position where normalization starts.
     * @param end       Sample position where normalization ends (inclusive).
     * @param handler   Error handler for parameter validation failures.
     * @param target    Normalization target (linear, not dBFS) in range [0.0f, 1.0f]. Default = 1.0f.
     * @param mode      Normalization mode (NormalizationMode::Peak or NormalizationMode::RMS). Default = Peak.
     * 
     * @return Shared pointer to a valid NormalizationParams instance, or nullptr if parameters are invalid.
     */
    static std::shared_ptr<NormalizationParams> create(sample_pos& start, sample_pos& end,
        AJ::error::IErrorHandler &handler, float target = 1.0f,
        NormalizationMode mode = NormalizationMode::Peak);

    /// @return The current target amplitude (linear).
    float Target(){
        return mTarget;
    }

    /**
     * @brief Set the target amplitude (linear scale).
     * @param factor Target amplitude in range [0.0f, 1.0f].
     */
    void setTarget(float factor){
        mTarget = std::clamp(factor, 0.0f, 1.0f);
    }

    /// @return The current normalization mode.
    NormalizationMode Mode(){
        return mMode;
    }

    /**
     * @brief Set the normalization mode.
     * @param mode NormalizationMode::Peak or NormalizationMode::RMS.
     */
    void setMode(NormalizationMode mode){
        mMode = mode;
    }

    /// @return The current gain factor.
    float Gain(){
        return mGain;
    }

    /**
     * @brief Set the gain factor.
     * @param gain Gain multiplier, clamped to [0.0f, 5.0f].
     */
    void setGain(float gain){
        mGain = std::clamp(gain, 0.0f, 5.0f);
    }

    /**
     * @brief Private constructor for controlled instantiation.
     */
    NormalizationParams(PrivateTag){
        mTarget = 1.0f;
        mMode = NormalizationMode::Peak;
    }
};

/**
 * @brief Normalization effect.
 *
 * Applies gain to audio samples to achieve a specified
 * target amplitude, either by peak normalization or RMS normalization.
 * In RMS mode, a peak clamp is applied to avoid clipping.
 *
 * TODO: Fix RMS Function.
 */
class Normalization : public AJ::dsp::Effect {

    std::shared_ptr<NormalizationParams> mParams; ///< Effect parameters.

    /**
     * @brief Applies gain using AVX vectorization without clamping.
     * 
     * Optimized for Peak normalization where clamping is unnecessary.  
     * Uses AVX instructions to process multiple samples in parallel.
     * 
     * @param buffer   Audio buffer to be modified in-place.
     * @param handler  Error handler for processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool gainAVX(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief Applies gain using a naive scalar loop without clamping.
     * 
     * Intended for Peak normalization when vectorization is not available  
     * or AVX optimization is disabled.
     * 
     * @param buffer   Audio buffer to be modified in-place.
     * @param handler  Error handler for processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool gainNaive(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief Dispatches to the appropriate gain function (AVX or naive).
     * 
     * Automatically selects between AVX and naive implementations  
     * based on platform capabilities and compile-time configuration.
     * 
     * @param buffer   Audio buffer to be modified in-place.
     * @param handler  Error handler for processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool gain(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief Performs RMS-based normalization.
     * 
     * Adjusts gain so that the Root Mean Square (RMS) level matches the target.  
     * Includes a basic peak limiter to reduce excessive gain.  
     * 
     * @warning Current implementation may produce unpredictable results  
     * for low-level or sparse audio. Peak normalization is recommended.
     * 
     * @param buffer   Audio buffer to be normalized in-place.
     * @param handler  Error handler for processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool normalizationRMS(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief Performs Peak-based normalization.
     * 
     * Scales the audio buffer so the highest absolute sample reaches the target.  
     * This method is fast, predictable, and safe for production use.
     * 
     * @param buffer   Audio buffer to be normalized in-place.
     * @param handler  Error handler for processing errors.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool normalizationPeak(Float &buffer, AJ::error::IErrorHandler &handler);

public:
    /// @brief Default constructor.
    Normalization(){
        mParams = nullptr;
    }

    /**
     * @brief Construct with parameters.
     * @param params   Normalization parameters.
     * @param handler  Error handler for validation.
     */
    Normalization(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
        setParams(params, handler);
    }

    /**
     * @brief Process the audio buffer using the selected normalization mode.
     * 
     * @param buffer   Audio samples to process.
     * @param handler  Error handler for reporting processing issues.
     * 
     * @return true if processing was successful, false otherwise.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Assigns normalization parameters to the effect.
     * 
     * Verifies that the passed `EffectParams` pointer is of type `NormalizationParams`.  
     * If type mismatch occurs, an error is reported via the handler and 
     * parameters are not updated.
     * 
     * @param params   Shared pointer to `NormalizationParams` object.
     * @param handler  Error handler for reporting validation failures.
     * 
     * @return true if parameters were successfully assigned, false otherwise.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;
};

}; // namespace AJ::dsp
