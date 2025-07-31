#pragma once
#include "audio_file.h"
extern "C"{
    #include <libavformat/avformat.h> 
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>  
    #include "libavutil/audio_fifo.h"
    #include <libswresample/swresample.h>
}

namespace AJ::io {
class MP3_File final : public AJ::io::AudioFile{

    void setAudioInfo(AVCodecContext *decoder_ctx, sample_c &total_samples);
public:
    MP3_File(){

    }

    bool read(AJ::error::IErrorHandler &handler) override;
    bool write(AJ::error::IErrorHandler &handler) override;
};
};