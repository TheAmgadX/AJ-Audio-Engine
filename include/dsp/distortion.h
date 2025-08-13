#pragma once
#include "effect.h"


namespace AJ::dsp {
class Distortion : public AJ::dsp::Effect {

// planning to add more distortion types.
enum DistortionType {
    SoftClipping
};

class DistortionParams : public EffectParams {
    
};


public:

    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

};
};