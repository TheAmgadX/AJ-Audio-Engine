#pragma once 
#include "portaudio.h"
#include "core/types.h"
#include "core/error_handler.h"
#include "audio_io.h"


namespace AJ::io::play {
class Player {

public:
    bool play(AJ::error::IErrorHandler& errHandler, AJ::io::IEventHandler& evHandler);
};    
}