#pragma once
#include "effect.h"
#include <algorithm>


namespace AJ::dsp {
class Reverb : public AJ::dsp::Effect {
private:
    void combFilter(Float &buffer, sample_pos start, sample_pos end);
    void allPassFilter(Float &buffer, sample_pos start, sample_pos end);

    void applyToneEQ(sample_t sample);
    void lowPass(sample_t sample);
    void highPass(sample_t sample);
    void setReverbParameters(); /* 
    function called before processing to calculate the gain and 
    the delay based on the room size and other affecting parameters:
            mDelay *= mRoomSize;
            mReverberance *= mRoomSize;
    */
protected:
    sample_c mDelay;

    float mRoomSize;
    float mPreDelay;

    float mReverberance;

    float mDamping;
    float mToneLow;
    float mToneHigh;

    float mDryMix;
    float mWetMix;
public: 

    Reverb(sample_c samplerate) : mRoomSize(0.5f), mPreDelay(0.01f),
        mReverberance(0.5f), mDamping(0.5f), mToneLow(0.5f),
        mToneHigh(0.5f), mDryMix(0.7f), mWetMix(0.3f) {

        mDelay = samplerate * 0.1;
    }

    // setters & getters
    void setRoomSize(float size) { 
        mRoomSize = std::clamp(size, 0.0f, 1.0f);
    }

    float roomSize() const { 
        return mRoomSize; 
    }

    void setPreDelay(float delay) { 
        mPreDelay = std::max(0.0f, delay); 
    }

    float preDelay() const { 
        return mPreDelay; 
    }

    void setReverberance(float value) { 
        mReverberance = std::clamp(value, 0.0f, 1.0f); 
    }

    float reverberance() const { 
        return mReverberance; 
    }

    void setDamping(float d) { 
        mDamping = std::clamp(d, 0.0f, 1.0f); 
    }

    float damping() const { 
        return mDamping; 
    }

    void setToneLow(float v) { 
        mToneLow = std::clamp(v, 0.0f, 1.0f); 
    }

    float toneLow() const { 
        return mToneLow; 
    }

    void setToneHigh(float v) {
         mToneHigh = std::clamp(v, 0.0f, 1.0f); 
    }

    float toneHigh() const { 
        return mToneHigh; 
    }

    void setDryMix(float v) { 
        mDryMix = std::clamp(v, 0.0f, 1.0f); 
    }

    float dryMix() const { 
        return mDryMix; 
    }

    void setWetMix(float v) { 
        mWetMix = std::clamp(v, 0.0f, 1.0f);
    }

    float wetMix() const { 
        return mWetMix;
    }

    void process(Float &buffer, sample_pos start, sample_pos end) override;
};
};