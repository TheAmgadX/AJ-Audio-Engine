#pragma once

#include<cstdint>
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp {
class Effect {
public:
    virtual bool process(Float &buffer, AJ::error::IErrorHandler &handler) = 0;
    virtual bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) = 0;
};
}