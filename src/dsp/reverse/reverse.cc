#include <iostream>
#include <memory>
#include <algorithm>

#include "core/types.h"
#include "core/errors.h"
#include "core/error_handler.h"

#include "dsp/reverse.h"
#include "core/effect_params.h"

std::shared_ptr<AJ::dsp::reverse::ReverseParams> AJ::dsp::reverse::ReverseParams::create(Params& params,
    AJ::error::IErrorHandler &handler){
    
    std::shared_ptr<ReverseParams> reverseParams = std::make_shared<ReverseParams>(PrivateTag{});
        
    if(params.mStart > params.mEnd || params.mStart < 0) {
        const std::string message = "invalid range indexes parameters for reverse effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return nullptr;
    }

    reverseParams->setStart(params.mStart);
    reverseParams->setEnd(params.mEnd);

    return reverseParams;
}

bool AJ::dsp::reverse::Reverse::setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) {
    std::shared_ptr<ReverseParams> reverseParams = std::dynamic_pointer_cast<ReverseParams>(params);
    // if reverseParams is nullptr that means it's not a shared_ptr of ReverseParams
    if(!reverseParams){
        const std::string message = "Effect parameters must be of type reverseParams for this effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    mParams = reverseParams; 
    return true;
}

bool AJ::dsp::reverse::Reverse::process(Float &buffer, AJ::error::IErrorHandler &handler){
    sample_c start = mParams->Start();
    sample_c end = mParams->End();

    if(end < start || start < 0 || start >= buffer.size() || end >= buffer.size()){
        const std::string message = "invalid indexes for reverse effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    std::reverse(buffer.begin() + start, buffer.begin() + end + 1);

    return true;
}


