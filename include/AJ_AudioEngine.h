#pragma once
#include <vector>
#include <stack>
#include <memory>

#include "AudioFile.h"


namespace aj {

template<typename T>
class AJ_Engine {
public:
    // DSP effects functions
    void Echo(std::vector<float>::iterator begin, std::vector<float>::iterator end,
        int delay_samples, float decay);

    void Reverb();
    void Gain();
    void Normalization();
    void Distortion();
    void PitchShift();
    void FadeIn();
    void FadeOut();
    void Reverse();

    // Functionalities
    void Cut();
    void Play();
    void Record();
    void Undo();
    void Redo();


    stack<string> operations;
    vector<std::unique_ptr<aj::io::AudioFile>> Audio_Files;
};

} // namespace AJ
