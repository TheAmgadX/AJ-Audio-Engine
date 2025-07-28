#include <iostream>
#include <limits>
#include <thread>

#include "dsp/echo.h"
#include "core/types.h"
#include "core/error_handler.h"

bool AJ::dsp::Echo::process(Float &buffer, AJ::error::IErrorHandler &handler) {
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return echoSIMD_AVX(buffer, handler);
    }
#endif

#if defined(__SSE__)
    if (__builtin_cpu_supports("sse")) {
        return echoSIMD_SSE(buffer, handler);
    }
#endif

    return echoNaive(buffer, handler);
}

AJ::sample_t AJ::dsp::Echo::calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx,  AJ::error::IErrorHandler &handler){
    if(sample_idx >= in.size() || sample_idx < 0){
        const std::string message = "Invalid Index: sample_idx is not in a valid range.\n";
        handler.onError(error::Error::InternalError, message);
        
        return 0.0f;
    }
    if(echo_idx >= in.size() || echo_idx < 0){       
        const std::string message = "Invalid Index: echo_idx is not in a valid range.\n";
        handler.onError(error::Error::InternalError, message);

        return 0.0f;
    }

    sample_t newSample = in[sample_idx] + (in[echo_idx] * mParams->mDecay);

    if(newSample > 1.0f) newSample = 1.0f;
    else if (newSample < -1.0f) newSample = -1.0f;

    return newSample;
}

void AJ::dsp::Echo::SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate){
    mParams->mDelaySamples = sampleRate * delayInSeconds;
}

bool AJ::dsp::Echo::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
    std::shared_ptr<EchoParams> echoParams = std::dynamic_pointer_cast<EchoParams>(params);
    // if echoParams is nullptr that means it's not a shared_ptr of EchoParams
    if(!echoParams){
        const std::string message = "Effect parameters must be of type EchoParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = echoParams; 
    return true;
}

bool AJ::dsp::Echo::echoNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    /*
        ?Echo Effect Equation:
            Echo_Sample = in[i - _mDelaySamples] * _mDecay
            out[i] = in[i] + Echo_Sample

        ! we must avoid writing at the input buffer to avoid speculative samples.
    */

    // check valid indexes ranges
    if(mParams->mEnd < mParams->mStart || mParams->mStart < 0 ||
         mParams->mStart + mParams->mDelaySamples >= buffer.size() || mParams->mEnd >= buffer.size()){
        const std::string message = "invalid indexes for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return false;
    }

    Float out;
    //* copy the first samples
    out.resize(mParams->mEnd - mParams->mStart + 1, 0.0f);
    std::copy(buffer.begin() + mParams->mStart, buffer.begin() + mParams->mStart + mParams->mDelaySamples, out.begin());

    for(sample_pos i = mParams->mStart + mParams->mDelaySamples; i <= mParams->mEnd; ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mParams->mDelaySamples, handler);
        out[i - mParams->mStart] = sample;
    }

    //* copy the new samples into the main buffer
    std::copy(out.begin(), out.end(), buffer.begin() + mParams->mStart);
    return true;
}



 