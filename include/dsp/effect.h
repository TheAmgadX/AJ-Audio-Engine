#pragma once

namespace AJ::dsp {
    class Effect {
    public:
        virtual void process() = 0;
    };
}