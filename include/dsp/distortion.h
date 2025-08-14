#pragma once
#include "effect.h"
#include <algorithm>
#include <memory>

#include "effect.h"
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp::distortion {

/**
 * @brief Enumeration of available distortion processing types.
 */
enum DistortionType {
    SoftClipping ///< Soft clipping distortion using tanh waveshaping.
};

/**
 * @brief Plain parameter structure for initializing distortion parameters.
 */
struct Params {
    sample_c mStart;           ///< Starting sample index for processing.
    sample_c mEnd;             ///< Ending sample index for processing.
    float mGain;               ///< Gain multiplier for distortion.
    DistortionType mType;      ///< Type of distortion to apply.
    
    /**
     * @brief Default constructor.
     *
     * Initializes the distortion type to SoftClipping. Other members
     * remain uninitialized and should be explicitly set before use.
     */
    Params(){ mType = DistortionType::SoftClipping; }
};

/**
 * @brief Parameter container for the Distortion effect.
 * 
 * Provides controlled creation and validation of distortion parameters.
 * Use the static create() method to instantiate this class.
 */
class DistortionParams : public EffectParams {
    struct PrivateTag {}; ///< Tag type to restrict public construction.
    sample_c mStart;      ///< Starting sample index.
    sample_c mEnd;        ///< Ending sample index.(inclusive)
    float mGain;          ///< Gain multiplier.
    DistortionType mType; ///< Distortion processing type.

public:

    /**
     * @brief Factory method to create a DistortionParams instance.
     * 
     * Constructs and validates an DistortionParams object using values provided in a Params structure.
     * The constructor is intentionally restricted — use this method as the only way
     * to create a DistortionParams instance.
     * 
     * @param params   Struct containing all Distortion effect parameters.
     * @param handler  Error handler for reporting parameter validation failures.
     * 
     * @return Shared pointer to a valid DistortionParams instance if parameters are valid,
     *         otherwise nullptr.
     */
    static std::shared_ptr<DistortionParams> create(Params &params, AJ::error::IErrorHandler &handler);

    /**
     * @brief Set the distortion gain.
     * 
     * @param gain Gain value, clamped between [0.0, 10.0].
     */
    void setGain(float gain) {
        mGain = std::clamp(gain, 0.1f, 10.0f);
    }

    /**
     * @brief Get the current gain value.
     * 
     * @return Gain multiplier.
     */
    float Gain() const noexcept {
        return mGain;
    }

    /**
     * @brief Set the distortion processing type.
     * 
     * @param type Distortion type.
     */
    void setType(DistortionType type) noexcept {
        mType = type;
    }

    /**
     * @brief Get the distortion processing type.
     * 
     * @return Distortion type.
     */
    DistortionType Type() const noexcept {
        return mType;
    }


    /**
     * @brief Private-tag constructor with default values.
     */
    DistortionParams(PrivateTag){
        mStart = -1;
        mEnd = -1;
        mGain = 1.0f;
        mType = DistortionType::SoftClipping;
    }
};

/**
 * @brief Distortion DSP effect class.
 * 
 * Applies various distortion algorithms to an audio buffer,
 * controlled by an associated DistortionParams object.
 * 
 * @note You must call setParams() with a valid DistortionParams instance
 *       before calling process(), otherwise processing will fail or
 *       produce undefined results.
 */
class Distortion : public AJ::dsp::Effect {
    std::shared_ptr<DistortionParams> mParams; ///< Effect parameters.

    /**
     * @brief Apply soft clipping distortion to the buffer.
     * 
     * @param buffer  Audio buffer to process.
     * @param handler Error handler for reporting invalid parameters.
     * 
     * @return true if processing succeeded, false otherwise.
     */
    bool softClipping(Float& buffer, AJ::error::IErrorHandler &handler);

public:

    /**
     * @brief Set effect parameters from a shared EffectParams object.
     * 
     * Downcasts to DistortionParams internally.
     * 
     * @param params  Shared pointer to EffectParams (expected DistortionParams).
     * @param handler Error handler for invalid parameter sets.
     * 
     * @return true if parameters were valid and applied.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Process the given audio buffer.
     * 
     * Chooses the distortion algorithm based on the DistortionType
     * set in the associated DistortionParams, applies it to the buffer,
     * and updates the buffer in-place with the processed audio.
     * 
     * @param buffer  Audio buffer to apply distortion to.
     * @param handler Error handler for reporting issues.
     * 
     * @return true if processing succeeded, false otherwise.
     * 
     * @note Requires that setParams() has been called with a valid
     *       DistortionParams instance before use.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    /** @name Parameter Accessors */
    ///@{

    /** @brief Set the start index for processing. */
    void setStart(sample_c start) { if (mParams) mParams->setStart(start); }

    /** @brief Get the start index for processing. */
    sample_c Start() const { return mParams ? mParams->Start() : -1; }

    /** @brief Set the end index for processing. */
    void setEnd(sample_c end) { if (mParams) mParams->setEnd(end); }

    /** @brief Get the end index for processing. */
    sample_c End() const { return mParams ? mParams->End() : -1; }

    /** @brief Set the distortion gain. */
    void setGain(float gain) { if (mParams) mParams->setGain(gain); }

    /** @brief Get the distortion gain. */
    float Gain() const { return mParams ? mParams->Gain() : 1.0f; }

    /** @brief Set the distortion type. */
    void setType(DistortionType type) { if (mParams) mParams->setType(type); }

    /** @brief Get the distortion type. */
    DistortionType Type() const { return mParams ? mParams->Type() : DistortionType::SoftClipping; }

    ///@}
};

} // namespace AJ::dsp::distortion
