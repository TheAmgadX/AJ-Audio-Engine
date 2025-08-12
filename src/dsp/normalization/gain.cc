#include <iostream>
 
#include "dsp/normalization.h"
#include "core/types.h"
#include "core/error_handler.h"

bool AJ::dsp::normalization::Normalization::gain(Float &buffer, AJ::error::IErrorHandler &handler){
    if(mParams->Gain() == 1.0f) return true;
    
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return gainAVX(buffer, handler);
    }
#endif

    return gainNaive(buffer, handler);
}

bool AJ::dsp::normalization::Normalization::gainNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 || 
        mParams->Start() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    for(sample_pos i = mParams->Start(); i <= mParams->End(); ++i){
        buffer[i] *= mParams->Gain();
    }

    return true;
}