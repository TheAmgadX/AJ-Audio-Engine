#pragma once
#include "effect.h"


namespace AJ::dsp {
    class PitchShift : public AJ::dsp::Effect {
    public:
        void process(AudioBuffer &buffer, sample_pos start, sample_pos end, short chan) override;

    };
};