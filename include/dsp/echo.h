#pragma once
#include "effect.h"
#include "types.h"

namespace AJ::dsp {
class Echo : public AJ::dsp::Effect {
private:

    /// @brief Process in different Implementations based on the Computer Architecture.
    /// @param in input audio channel buffer.
    /// @param out output audio channel buffer.
    /// @param start start processing at index.
    /// @param end end processing at index.
    void EchoNaive(Float &in, Float &out, sample_pos start, sample_pos end);
    void EchoSIMD_8(Float &in, Float &out, sample_pos start, sample_pos end);
    void EchoSIMD_16(Float &in, Float &out, sample_pos start, sample_pos end);
    void EchoSIMD_32(Float &in, Float &out, sample_pos start, sample_pos end);

    /// @brief used to calculate the sample and make sure it's in a valid range
    /// @param in input channel buffer
    /// @param idx current sample index
    /// @param echo_idx delay sample index
    /// @return the new sample
    sample_t CalculateNewSampleWithEcho(Float &in, sample_pos sample_idx, sample_pos echo_idx);

    decay_t _mDecay;
    sample_c _mDelaySamples;

public:
    void process() override;

    decay_t GetDecay(){
        return _mDecay;
    }

    void SetDecay(decay_t decay){
        _mDecay = decay;
    } 

    sample_c GetDelaySampels(){
        return _mDelaySamples;
    }

    void SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate);

};
};