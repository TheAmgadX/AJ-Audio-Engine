#include <iostream>
#include <limits>
#include <thread>

#include "dsp/echo.h"
#include "core/types.h"
#include "core/error_handler.h"

std::shared_ptr<AJ::dsp::echo::EchoParams> AJ::dsp::echo::EchoParams::create(Params& params,
    AJ::error::IErrorHandler &handler) {
    std::shared_ptr<EchoParams> echoParams = std::make_shared<EchoParams>(PrivateTag{});
    
    sample_c delaySamples = params.mDelayInSeconds * params.mSamplerate;

    if(params.mStart > params.mEnd || params.mStart < 0 || params.mStart + delaySamples >= params.mEnd) {
        const std::string message = "invalid range indexes parameters for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    echoParams->setDecay(params.mDecay);
    echoParams->setDelaySamples(delaySamples);
    echoParams->setStart(params.mStart);
    echoParams->setEnd(params.mEnd);

    return echoParams;
}

bool AJ::dsp::echo::Echo::process(Float &buffer, AJ::error::IErrorHandler &handler) {
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

AJ::sample_t AJ::dsp::echo::Echo::calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx,  AJ::error::IErrorHandler &handler){
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

    sample_t newSample = in[sample_idx] + (in[echo_idx] * mParams->Decay());

    if(newSample > 1.0f) newSample = 1.0f;
    else if (newSample < -1.0f) newSample = -1.0f;

    return newSample;
}

void AJ::dsp::echo::Echo::SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate){
    mParams->setDelaySamples(sampleRate * delayInSeconds);
}

bool AJ::dsp::echo::Echo::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler){
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

bool AJ::dsp::echo::Echo::echoNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    /*
        ?Echo Effect Equation:
            Echo_Sample = in[i - _DelaySamples()] * _mDecay
            out[i] = in[i] + Echo_Sample

        ! we must avoid writing at the input buffer to avoid speculative samples.
    */

    // check valid indexes ranges
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 ||
         mParams->Start() + mParams->DelaySamples() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for echo effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return false;
    }

    Float out;
    //* copy the first samples
    out.resize(mParams->End() - mParams->Start() + 1, 0.0f);
    std::copy(buffer.begin() + mParams->Start(), buffer.begin() + mParams->Start() + mParams->DelaySamples(), out.begin());

    for(sample_pos i = mParams->Start() + mParams->DelaySamples(); i <= mParams->End(); ++i){
        sample_t sample = calculate_new_sample_with_echo(buffer, i, i - mParams->DelaySamples(), handler);
        out[i - mParams->Start()] = sample;
    }

    //* copy the new samples into the main buffer
    std::copy(out.begin(), out.end(), buffer.begin() + mParams->Start());
    return true;
}



 