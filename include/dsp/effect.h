#pragma once

#include<cstdint>
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp {
/// @brief Base interface for all DSP audio effects.
///
/// Any audio effect that modifies audio buffers must inherit from this class.
/// This class provides a common interface for applying effects and passing configuration parameters.
class Effect {
public:
    /// @brief Apply the effect to the given audio buffer.
    /// 
    /// @param buffer The audio buffer to be processed in-place.
    /// @param handler Error handler callback used to report any processing failures.
    /// @return true if processing was successful, false if an error occurred.
    virtual bool process(Float &buffer, AJ::error::IErrorHandler &handler) = 0;

    /// @brief Set the parameters for the effect.
    ///
    /// Each effect has its own derived EffectParams subclass that must be passed.
    /// @param params Shared pointer to an EffectParams-derived object with relevant parameters.
    /// @param handler Error handler callback for reporting invalid or missing parameters.
    /// @return true if parameters were successfully set, false otherwise.
    virtual bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) = 0;

    virtual ~Effect() = default;
};

}