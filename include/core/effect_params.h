#pragma once

namespace AJ::dsp {
class EffectParams {
public:
   virtual ~EffectParams() = default;
   sample_pos mStart, mEnd;
};
}