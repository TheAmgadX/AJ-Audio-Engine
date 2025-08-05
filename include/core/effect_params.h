#pragma once

namespace AJ::dsp {
/**
 * @brief Base class for effect parameter containers.
 * 
 * This abstract class serves as a base for passing parameters to different
 * audio effects. Each effect should define its own subclass that inherits from
 * EffectParams and adds specific parameters as needed.
 * 
 * Common parameters like `mStart` and `mEnd` define the part of the audio
 * which the effect should be applied to.
 */
class EffectParams {
public:
    virtual ~EffectParams() = default;

    /// @brief Start position of the effect in samples.
    sample_pos mStart;

    /// @brief End position (included) of the effect in samples.
    sample_pos mEnd;
};

}