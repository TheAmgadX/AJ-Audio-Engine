#pragma once
#include <list>
#include <memory>
#include <string>


#include "file_io/audio_file.h"

#include "dsp/cut.h"
#include "dsp/distortion.h"
#include "dsp/echo.h"
#include "dsp/fadeIn.h"
#include "dsp/fadeOut.h"
#include "dsp/gain.h"
#include "dsp/mixer.h"
#include "dsp/normalization.h"
#include "dsp/pitchShift.h"
#include "dsp/reverb.h"
#include "dsp/reverse.h"



namespace AJ {

template<typename T>

class AJ_Engine {
public:
    // DSP effects functions
    AJ::dsp::Echo mEcho;
    AJ::dsp::Reverb mReverb;
    AJ::dsp::Distortion mDistortion;
    AJ::dsp::Gain mGain;
    AJ::dsp::FadeIn mFadeIn;
    AJ::dsp::FadeOut mFadeOut;
    AJ::dsp::Normalization mNoNormalization;
    AJ::dsp::PitchShift mPitchShift;


    // Functionalities
    AJ::dsp::Cut mCut;
    AJ::dsp::Reverse mReverse;
    AJ::dsp::Mixer mMixer;

    
    // Engine Members
    std::List<Audio> mAudioFiles;
};
} // namespace AJ
