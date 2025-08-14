#pragma once
<<<<<<< Updated upstream
#include "effect.h"
=======
>>>>>>> Stashed changes

#include <algorithm>
#include <memory>

<<<<<<< Updated upstream
namespace AJ::dsp {
class Distortion : public AJ::dsp::Effect {

=======
#include "effect.h"
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp::distortion {

>>>>>>> Stashed changes
// planning to add more distortion types.
enum DistortionType {
    SoftClipping
};

<<<<<<< Updated upstream
class DistortionParams : public EffectParams {
    
};


public:

    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

=======
struct Params {
    sample_c mStart;
    sample_c mEnd;
    float mGain;
    DistortionType mType;
};

class DistortionParams : public EffectParams {
    struct PrivateTag {};
    sample_c mStart;
    sample_c mEnd;
    float mGain;
    DistortionType mType;

public:

    /**
     * @brief Factory method to create an DistortionParams instance.
     * 
     * Constructs and validates an DistortionParams object using values provided in a Params structure.
     * The constructor is intentionally restricted — use this method as the only way
     * to create an DistortionParams instance.
     * 
     * @param params   Struct containing all Distortion effect parameters.
     * @param handler  Error handler for reporting parameter validation failures.
     * 
     * @return Shared pointer to a valid DistortionParams instance if parameters are valid,
     *         otherwise nullptr.
     */
    static std::shared_ptr<DistortionParams> create(Params &params, AJ::error::IErrorHandler &handler);

    void setGain(float gain) {
        mGain = std::clamp(gain, 0.0f, 10.0f);
    }

    float Gain() const noexcept {
        return mGain;
    }

    void setType(DistortionType type) noexcept {
        mType = type;
    }

    DistortionType Type() const noexcept {
        return mType;
    }

    DistortionParams(PrivateTag){
        mStart = -1;
        mEnd = -1;
        mGain = 1.0f;
        mType = DistortionType::SoftClipping;
    }
};

class Distortion : public AJ::dsp::Effect {
    std::shared_ptr<DistortionParams> mParams;

    bool softClipping(Float& buffer, AJ::error::IErrorHandler &handler);
public:

    /**
     * @brief Set effect parameters from a shared EffectParams object.
     *        Will downcast to EchoParams.
     * 
     * @param params Shared pointer to DistortionParams.
     * @param handler Error handler.
     * 
     * @return true if parameters were valid and applied.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;
>>>>>>> Stashed changes
};
};