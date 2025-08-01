#include "file_io/mp3_file.h"
#include "core/error_handler.h"
#include "core/types.h"

extern "C"{
    #include <libavformat/avformat.h> 
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>  
    #include "libavutil/audio_fifo.h"
    #include <libswresample/swresample.h>
}

void AJ::io::MP3_File::setAudioInfo(AVCodecContext *decoder_ctx, sample_c &total_samples){
    mInfo.samplerate = decoder_ctx->sample_rate;
    mInfo.channels = decoder_ctx->channels;
    mInfo.length = total_samples / decoder_ctx->channels;
    mInfo.format = "mp3";
}

bool AJ::io::MP3_File::openFile(AJ::error::IErrorHandler &handler){
    mDecoderInfo.format_ctx = avformat_alloc_context();

    // open file and allocate format_ctx:
    // return < 0 when fail
    if(avformat_open_input(&mDecoderInfo.format_ctx, mFilePath.c_str(), nullptr, nullptr) < 0) { 
        const std::string message = "Couldn't open file: " + mFilePath + "\n";
        handler.onError(error::Error::FileOpenError, message);
        return false;
    }

    // retrieve stream info:
    if(avformat_find_stream_info(mDecoderInfo.format_ctx, nullptr) < 0){
        const std::string message = "Couldn't find stream info for the file: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);

        return false;
    }

    // find audio stream:
    mDecoderInfo.stream_idx = av_find_best_stream(mDecoderInfo.format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    if(mDecoderInfo.stream_idx < 0){
        const std::string message = "Couldn't find audio stream in: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);

        return false;
    }

    return true;
}

bool AJ::io::MP3_File::initDecoder(AJ::error::IErrorHandler &handler){
    // find and open decoder:
    mDecoderInfo.decoder_params = mDecoderInfo.format_ctx->streams[mDecoderInfo.stream_idx]->codecpar;

    mDecoderInfo.decoder = avcodec_find_decoder( mDecoderInfo.decoder_params->codec_id); 
    if(!mDecoderInfo.decoder){
        const std::string message = "Couldn't find decoder in: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);
        avcodec_parameters_free(&mDecoderInfo.decoder_params);

        return false; 
    }  

    mDecoderInfo.decoder_ctx = avcodec_alloc_context3(mDecoderInfo.decoder);

    if(!mDecoderInfo.decoder_ctx){
        const std::string message = "Couldn't initialize decoder context in: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);
        avcodec_parameters_free(&mDecoderInfo.decoder_params);

        return false;  
    }

    // copy info about the stream into decoder context:
    if(avcodec_parameters_to_context(mDecoderInfo.decoder_ctx, mDecoderInfo.decoder_params) < 0){
        const std::string message = "Couldn't copy stream parameters to decoder context for the file: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);
        avcodec_free_context(&mDecoderInfo.decoder_ctx);
        avcodec_parameters_free(&mDecoderInfo.decoder_params);

        return false;
    }

    // some files don't store the channel layout correctly.
    if (mDecoderInfo.decoder_ctx->channel_layout == 0)
        mDecoderInfo.decoder_ctx->channel_layout = av_get_default_channel_layout(mDecoderInfo.decoder_ctx->channels);
    
    // only support mono and stereo.
    if(mDecoderInfo.decoder_ctx->channels > 2){
        const std::string message = "Unsupported channels number, the Engine only support mono and stereo.\n";
        handler.onError(error::Error::UnsupportedFileFormat, message);
        avformat_close_input(&mDecoderInfo.format_ctx);
        avcodec_parameters_free(&mDecoderInfo.decoder_params);
        avcodec_free_context(&mDecoderInfo.decoder_ctx);

        return false;    
    }

    
    // open decoder:
    if(avcodec_open2(mDecoderInfo.decoder_ctx, mDecoderInfo.decoder, nullptr) < 0){
        const std::string message = "Couldn't open decoder for the file: " + mFilePath + "\n";
        handler.onError(error::Error::FileReadError, message);
        avformat_close_input(&mDecoderInfo.format_ctx);
        avcodec_free_context(&mDecoderInfo.decoder_ctx);
        avcodec_parameters_free(&mDecoderInfo.decoder_params);

        return false;
    }

    return true;
}

bool AJ::io::MP3_File::decode(AJ::error::IErrorHandler &handler){
// packet is storing chunk of compressed frames
    // we need to decode these samples before storing it.
    AVPacket *packet = av_packet_alloc();

    // frame is to store the decompressed packet data (raw data).
    AVFrame *frame = av_frame_alloc();

    // create a resampler to convert to float planar so we can get samples per channel in float type:
    SwrContext *resampler = swr_alloc_set_opts(nullptr, 
        mDecoderInfo.decoder_ctx->channel_layout, // output channel.
        AV_SAMPLE_FMT_FLTP, // output format (float planar)
        mDecoderInfo.decoder_params->sample_rate, // output samplerate.
        mDecoderInfo.decoder_ctx->channel_layout, // input cahnnel layout.
        (AVSampleFormat)frame->format, // input format.
        mDecoderInfo.decoder_params->sample_rate, // input sample rate.
        0, nullptr
    );

    // init a Fifo with 1024 nb_samples as starting point it may grow at runtime. 
    mDecoderInfo.fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLTP, mDecoderInfo.decoder_ctx->channels, 1024);
    
    // av_read_frame will return 0 if it was able to read compressed packet.
    while(av_read_frame(mDecoderInfo.format_ctx, packet) == 0) {
        // skip non audio packets if there.
        if(packet->stream_index != mDecoderInfo.stream_idx) continue;
        
        // send packet to the decoder
        int ret = avcodec_send_packet(mDecoderInfo.decoder_ctx, packet);
        
        if(ret < 0){
            // AVERROR (EAGAIN) ==> send the packet again after getting all frames out of decoder.
            if(ret != AVERROR(EAGAIN)){
                const std::string message = "Couldn't decode packets for the file: " + mFilePath + "\n";
                handler.onError(error::Error::FileReadError, message);
                avformat_close_input(&mDecoderInfo.format_ctx);
                avcodec_free_context(&mDecoderInfo.decoder_ctx);
                av_frame_free(&frame);
                swr_free(&resampler);

                return false;  
            }
        }
        
        
        while((ret = avcodec_receive_frame(mDecoderInfo.decoder_ctx, frame)) == 0){
            // resample the frame:
            AVFrame *resampled = av_frame_alloc();
            resampled->channel_layout = frame->channel_layout;
            resampled->channels = frame->channels;
            resampled->format = AV_SAMPLE_FMT_FLTP; // float planar.
            resampled->sample_rate = frame->sample_rate;

            ret = swr_convert_frame(resampler, resampled, frame);
            if(ret < 0){
                const std::string message = "Couldn't resample the frame for the file: " + mFilePath + "\n";
                handler.onError(error::Error::FileReadError, message);
                avformat_close_input(&mDecoderInfo.format_ctx);
                avcodec_free_context(&mDecoderInfo.decoder_ctx);
                avcodec_parameters_free(&mDecoderInfo.decoder_params);
                av_packet_free(&packet);
                av_frame_free(&frame);
                av_frame_free(&resampled);
                swr_free(&resampler);
                av_audio_fifo_free(mDecoderInfo.fifo);

                return false; 
            }

            av_frame_unref(frame); // free the memory of frame.

            int written_samples = av_audio_fifo_write(mDecoderInfo.fifo, (void**)resampled->data, resampled->nb_samples);
            if(written_samples != resampled->nb_samples){
                const std::string message = "Couldn't write samples into fifo buffer.\n";
                handler.onError(error::Error::FileReadError, message);
                avformat_close_input(&mDecoderInfo.format_ctx);
                avcodec_free_context(&mDecoderInfo.decoder_ctx);
                av_frame_free(&frame);
                swr_free(&resampler);
                av_frame_free(&resampled); 
                av_audio_fifo_free(mDecoderInfo.fifo);

                return false;  
            }

            mDecoderInfo.total_samples += resampled->nb_samples;
            av_frame_free(&resampled);
        }
        av_packet_unref(packet); // free the packet after each iteration
    }
    
    setAudioInfo(mDecoderInfo.decoder_ctx, mDecoderInfo.total_samples);
    
    // free the memory and destroy objects.
    avformat_close_input(&mDecoderInfo.format_ctx);
    avcodec_parameters_free(&mDecoderInfo.decoder_params);
    av_frame_free(&frame);
    swr_free(&resampler);
    av_packet_free(&packet);

    return true;
}

bool AJ::io::MP3_File::read(AJ::error::IErrorHandler &handler){
    //* 1. open file and initialize decoder:
    if(!openFile(handler)){
        return false;
    }

    if(!initDecoder(handler)){
        return false;
    }

    //* 2. start decoding
    
    if(!decode(handler)){
        return false;
    }

    //* 3. start writing to the audio buffer.

    // mono copy
    if(mDecoderInfo.decoder_ctx->channels == 1){
        avcodec_free_context(&mDecoderInfo.decoder_ctx);

        Float buffer(mDecoderInfo.total_samples);
        sample_c read_samples = av_audio_fifo_read(mDecoderInfo.fifo,
            (void **)&buffer, mDecoderInfo.total_samples);

        av_audio_fifo_free(mDecoderInfo.fifo);

        if(read_samples != mDecoderInfo.total_samples){
            const std::string message = "Couldn't read samples from the buffer.\n";
            handler.onError(error::Error::FileReadError, message);
            return false;  
        }
        pAudio->at(0) = buffer;
        return true;
    }

    avcodec_free_context(&mDecoderInfo.decoder_ctx);

    mDecoderInfo.total_samples /= 2;
    Float buffer_l(mDecoderInfo.total_samples);
    Float buffer_r(mDecoderInfo.total_samples);
    void *channels[2];

    channels[0] = buffer_l.data();
    channels[1] = buffer_r.data();

    sample_c read_samples = av_audio_fifo_read(mDecoderInfo.fifo, channels, mDecoderInfo.total_samples);
    av_audio_fifo_free(mDecoderInfo.fifo);
    
    if(read_samples != mDecoderInfo.total_samples){
        const std::string message = "Couldn't read samples from the buffer.\n";
        handler.onError(error::Error::FileReadError, message);

        return false;  
    }

    pAudio->at(0) = std::move(buffer_l);
    pAudio->at(1) = std::move(buffer_r);

    return true;
}

bool AJ::io::MP3_File::write(AJ::error::IErrorHandler &handler){




    return true;
}
