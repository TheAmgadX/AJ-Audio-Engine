#pragma once

#include<cstdint>
#include "core/types.h"


namespace AJ::dsp {
    class Effect {
    public:
        virtual void process(AudioBuffer &buffer, sample_pos start, sample_pos end, short chan) = 0;
        uint8_t mBitDepth;
    };
}