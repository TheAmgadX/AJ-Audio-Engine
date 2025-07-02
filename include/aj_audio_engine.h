#pragma once
#include <vector>
#include <stack>
#include <memory>

#include "audio_file.h"


namespace aj {

template<typename T>
class AJ_Engine {
public:
    // DSP effects functions
    void echo(std::vector<float>::iterator begin, std::vector<float>::iterator end,
        int delay_samples, float decay);

    void reverb();
    void gain();
    void normalization();
    void distortion();
    void pitchShift();
    void fadeIn();
    void fadeOut();
    void reverse();

    // Functionalities
    void cut();
    void play();
    void record();
    void undo();
    void redo();


    stack<string> operations;
    vector<std::unique_ptr<aj::io::AudioFile>> audio_Files;
};

} // namespace AJ
