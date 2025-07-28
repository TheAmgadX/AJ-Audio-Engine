#pragma once
#include "effect.h"
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp {

class EchoParams : public EffectParams {
public:
    float mDecay;
    sample_c mDelaySamples;

    ~EchoParams() override = default;
    
    EchoParams(){
        mStart = -1;
        mEnd = -1;
    }

    EchoParams(sample_pos start, sample_pos end){
        mStart = start;
        mEnd = end;
    }
};

class Echo : public AJ::dsp::Effect {
private:

    /// @brief Process in different Implementations based on the Computer Architecture.
    /// @param in input audio channel buffer.
    /// @param out output audio channel buffer.
    /// @param start start processing at index.
    /// @param end end processing at index.
    bool echoNaive(Float &buffer, AJ::error::IErrorHandler &handler);
    bool echoSIMD_SSE(Float &buffer, AJ::error::IErrorHandler &handler);
    bool echoSIMD_AVX(Float &buffer, AJ::error::IErrorHandler &handler);

    /// @brief used to calculate the sample and make sure it's in a valid range
    /// @param in input channel buffer
    /// @param idx current sample index
    /// @param echo_idx delay sample index
    /// @return the new sample
    sample_t calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx, AJ::error::IErrorHandler &handler);
    std::shared_ptr<EchoParams> mParams;

public:
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    Echo(){
        mParams = std::make_shared<EchoParams>();
    }

    decay_t GetDecay(){
        return mParams->mDecay;
    }

    void SetDecay(decay_t decay){
        mParams->mDecay = decay;
    } 

    sample_c GetDelaySampels(){
        return mParams->mDelaySamples;
    }

    void setRange(sample_pos start, sample_pos end){
        if(start <= end){
            mParams->mStart = start;
            mParams->mEnd = end;
        }
    }

    void SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate);
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;
};
};