#include <iostream>
#include "core/aj_audio_engine.h"

int main(){
    // you can just use: auto engine = AJ::AJ_Engine::create();
    std::shared_ptr<AJ::AJ_Engine> engine = AJ::AJ_Engine::create();

    const std::string path = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio/violin.wav";
    const std::string ext = "wav";
    AJ::error::ConsoleErrorHandler handler;

    std::cout << "start reaing wav file.\n";
    // auto audio = engine->loadAudio(path, ext, handler);
    std::shared_ptr<AJ::io::AudioFile> audio = engine->loadAudio(path, ext, handler);

    if(audio){
        std::cout << "file reading done successfully.\n";
    } else {
        return 0;
    }

    // std::shared_ptr<AJ::dsp::reverb::ReverbParams> params;
    auto params = std::make_shared<AJ::dsp::reverb::ReverbParams>();

    params->mDelayMS = 40.0f;
    params->mDryMix = 0.3f;
    params->mWetMix = 0.7f;
    params->mGain = 0.7f;
    params->mSamplerate = audio->mInfo.samplerate;
    params->mStart = 0;
    params->mEnd = (audio->mInfo.length / audio->mInfo.channels) - 1; // end is included.

    std::cout << "start processing reverb effect.\n";
    if(engine->applyEffect(audio->pAudio->at(0), AJ::Effect::reverb, params, handler)){
        std::cout << "first channel processed successfully.\n";
    } else {
        return 0;
    }

    if(audio->mInfo.channels == 2){ // stereo
        if(engine->applyEffect(audio->pAudio->at(1), AJ::Effect::reverb, params, handler)){
            std::cout << "second channel processed successfully.\n";
        } else {
            return 0;
        }
    }

    AJ::AudioWriteInfo writeInfo;
    writeInfo.bitdepth = audio->mInfo.bitdepth;
    writeInfo.samplerate = audio->mInfo.samplerate;
    writeInfo.channels = audio->mInfo.channels;
    writeInfo.length = audio->mInfo.length; // the length here is the samples for all channels.
    writeInfo.seekable = audio->mInfo.seekable;
    writeInfo.format = audio->mInfo.format;
    writeInfo.name = "reverbed_violin";
    writeInfo.path = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/audio/";

    audio->setWriteInfo(writeInfo, handler);

    std::cout << "start saving new file.\n";
    if(engine->saveAudio(audio, handler)){
        std::cout << "new file saved successfully.\n";
    }
}