#pragma once
#include "effect.h"
#include "core/types.h"

namespace AJ::dsp {
class Echo : public AJ::dsp::Effect {
private:

    /// @brief Process in different Implementations based on the Computer Architecture.
    /// @param in input audio channel buffer.
    /// @param out output audio channel buffer.
    /// @param start start processing at index.
    /// @param end end processing at index.
    void echoNaive(Float &buffer, sample_pos start, sample_pos end);
    void echoSIMD_SSE(Float &buffer, sample_pos start, sample_pos end);
    void echoSIMD_AVX(Float &buffer, sample_pos start, sample_pos end);

    /// @brief used to calculate the sample and make sure it's in a valid range
    /// @param in input channel buffer
    /// @param idx current sample index
    /// @param echo_idx delay sample index
    /// @return the new sample
    sample_t calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx);
    decay_t mDecay;
    sample_c mDelaySamples;

public:
    void process(AudioBuffer &buffer, sample_pos start, sample_pos end, short chan) override;

    decay_t GetDecay(){
        return mDecay;
    }

    void SetDecay(decay_t decay){
        mDecay = decay;
    } 

    sample_c GetDelaySampels(){
        return mDelaySamples;
    }

    void SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate);

};
};