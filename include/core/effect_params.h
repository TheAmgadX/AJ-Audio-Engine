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
private:
    /// @brief Start position of the effect in samples.
    sample_pos mStart;

    /// @brief End position (included) of the effect in samples.
    sample_pos mEnd;
public:
    virtual ~EffectParams() = default;

    /// @brief Set the start position of the effect in samples.
    void setStart(sample_pos start) { 
        mStart = start; 
    }

    /// @brief Get the start position of the effect in samples.
    sample_pos Start() const { 
        return mStart; 
    }

    /// @brief Set the end position (inclusive) of the effect in samples.
    void setEnd(sample_pos end) { 
        mEnd = end; 
    }

    /// @brief Get the end position (inclusive) of the effect in samples.
    sample_pos End() const { 
        return mEnd; 
    }
};

}