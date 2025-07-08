#pragma once

namespace AJ::dsp {
    class Effect {
    public:
        virtual void process(AudioBufferBlocks &buffer, sample_pos start, sample_pos end) = 0;
        uint8_t mBitDepth;
    };
}