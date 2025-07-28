#include <list>
#include <stack>

#include <memory>
#include <string>

#include "core/aj_audio_engine.h"

#include "file_io/audio_file.h"
#include "file_io/wav_file.h"
#include "file_io/mp3_file.h"

// dsp
#include "dsp/effect.h"
#include "dsp/gain.h"
#include "dsp/echo.h"
#include "dsp/reverb/reverb.h"

#include "core/effect_params.h"
#include "core/error_handler.h"

#include "undo_system/state.h"
#include "undo_system/undo.h"

std::shared_ptr<AJ::AJ_Engine> AJ::AJ_Engine::create(){
    auto engine = std::make_shared<AJ_Engine>();
    return engine;
}

std::shared_ptr<AJ::io::AudioFile> AJ::AJ_Engine::loadAudio(const std::string &path,
    const std::string &ext, error::IErrorHandler &handler){

    std::shared_ptr<io::AudioFile> audio;
    
    if(ext == "wav" || ext == "WAV"){
        audio = std::make_shared<io::WAV_File>(); 
    } else if (ext == "mp3" || ext == "MP3"){
        audio = std::make_shared<io::MP3_File>(); 
    } else {
        const std::string message = "Audio format not recognized. Please ensure the file is in WAV or MP3 format.\n";
        handler.onError(error::Error::UnsupportedFileFormat, message);
        return nullptr;
    }

    if(!audio->setFilePath(path)){
        const std::string message = "Failed to validate file path. Please provide a valid file location.\n";
        handler.onError(error::Error::InvalidFilePath, message);
        return nullptr;
    }    

    if(!audio->setFileExtension(ext)){
        const std::string message = "Unsupported audio format. The engine currently supports WAV and MP3 formats only.\n";
        handler.onError(error::Error::UnsupportedFileFormat, message);
        return nullptr;
    }

    if(!audio->read(handler)){
        return nullptr;
    }

    return audio;
}

bool AJ::AJ_Engine::saveAudio(std::shared_ptr<io::AudioFile> audio, error::IErrorHandler &handler){
    if(!audio->write(handler)){
        return false;
    }

    return true;
}

bool AJ::AJ_Engine::applyEffect(Float &buffer, const Effect &effect,
    std::shared_ptr<dsp::EffectParams> params, error::IErrorHandler &handler){
    
    std::shared_ptr<AJ::dsp::Effect> audioEffect;

    switch (effect)
    {
        case Effect::gain: {
            audioEffect = std::make_shared<AJ::dsp::Gain>();
            break;
        }

        case Effect::echo: {
            audioEffect = std::make_shared<AJ::dsp::Echo>();
            break;
        }

        case Effect::reverb: {
            audioEffect = std::make_shared<AJ::dsp::reverb::Reverb>();
            break;
        }

        case Effect::fadeIn: {
            const std::string message = "Fade in effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        case Effect::fadeOut: {
            const std::string message = "Fade out effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        case Effect::normalization: {
            const std::string message = "Normalization effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        case Effect::pitchShift: {
            const std::string message = "Pitch shift effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        case Effect::reverse: {
            const std::string message = "Reverse effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        case Effect::Distortion: {
            const std::string message = "Distortion effect is not implemented yet.\n";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }

        default: {
            const std::string message = "Unknown or unsupported audio effect requested.";
            handler.onError(error::Error::UnknownEffect, message);
            return false;
        }
    }


    if (!audioEffect->setParams(params, handler)) {
        return false;
    }

    if (!audioEffect->process(buffer, handler)) {
        return false;  
    }

    return true;
}
