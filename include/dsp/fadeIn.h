#pragma once
#include "effect.h"


namespace AJ::dsp {
    class FadeIn : public AJ::dsp::Effect {
    public:
        void process(AudioBufferBlocks &buffer, sample_pos start, sample_pos end) override {

        }
    };
};