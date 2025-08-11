#include <iostream>
 
#include "dsp/normalization.h"
#include "core/types.h"
#include "core/error_handler.h"

bool AJ::dsp::Normalization::gain(Float &buffer, AJ::error::IErrorHandler &handler){
    if(mParams->Gain() == 1.0f) return true;
    
#if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        return gainAVX(buffer, handler);
    }
#endif

    return gainNaive(buffer, handler);
}

bool AJ::dsp::Normalization::gainNaive(Float &buffer, AJ::error::IErrorHandler &handler){
    if(mParams->mEnd < mParams->mStart || mParams->mStart < 0 || 
        mParams->mStart >= buffer.size() || mParams->mEnd >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    for(sample_pos i = mParams->mStart; i <= mParams->mEnd; ++i){
        buffer[i] *= mParams->Gain();
    }

    return true;
}