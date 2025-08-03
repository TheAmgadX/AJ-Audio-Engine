#include <cmath>

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

void AJ::io::MP3_File::setAudioInfo(AVCodecContext *decoder_ctx, sample_c &total_samples_per_chan){
    mInfo.samplerate = decoder_ctx->sample_rate;
    mInfo.channels = decoder_ctx->ch_layout.nb_channels;
    mInfo.length = total_samples_per_chan * decoder_ctx->ch_layout.nb_channels;
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
    if (mDecoderInfo.decoder_ctx->ch_layout.nb_channels == 0)
        av_channel_layout_default(&mDecoderInfo.decoder_ctx->ch_layout,
            mDecoderInfo.decoder_ctx->ch_layout.nb_channels);
    
    // only support mono and stereo.
    if(mDecoderInfo.decoder_ctx->ch_layout.nb_channels > 2){
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

bool AJ::io::MP3_File::decode(AJ::error::IErrorHandler &handler) {
    // allocate packet.
    AVPacket *packet = av_packet_alloc();
    
    // init frame.
    AVFrame *frame = av_frame_alloc();
    
    frame->format = mDecoderInfo.decoder_params->format;
    frame->sample_rate = mDecoderInfo.decoder_params->sample_rate;
    frame->ch_layout = mDecoderInfo.decoder_params->ch_layout;
    frame->nb_samples = mDecoderInfo.decoder_params->frame_size;

    // Create resampler
    SwrContext *resampler = swr_alloc();
    const AVChannelLayout *chan_layout = &mDecoderInfo.decoder_ctx->ch_layout;

    int ret = swr_alloc_set_opts2(&resampler, 
        chan_layout,
        AV_SAMPLE_FMT_FLTP,
        mDecoderInfo.decoder_params->sample_rate,
        chan_layout,
        (AVSampleFormat)frame->format,
        mDecoderInfo.decoder_params->sample_rate,
        0, nullptr
    );

    if(ret != 0 || swr_init(resampler) != 0) {
        const std::string message = "Couldn't initialize audio resampler.\n";
        handler.onError(error::Error::FileWriteError, message);
        swr_free(&resampler);
        return false;
    }

    /*
    * Read audio in fixed-size chunks to prevent AVAudioFifo from exceeding its capacity
    * or causing realloc failures.
    * Each chunk is appended to temporary buffers for efficient accumulation.
    * Once the entire file is processed, move the accumulated data into the pAudio buffers.
    */

    // Use 1MB chunk size.
    const size_t CHUNK_SIZE = (1024 * 1024) / (sizeof(float) * mDecoderInfo.decoder_ctx->ch_layout.nb_channels);
    
    // Initialize vectors for each channel
    std::vector<std::vector<float>> temp_buffers(mDecoderInfo.decoder_ctx->ch_layout.nb_channels);
    
    // Initialize FIFO for chunk processing
    mDecoderInfo.fifo = av_audio_fifo_alloc(
        AV_SAMPLE_FMT_FLTP,
        mDecoderInfo.decoder_ctx->ch_layout.nb_channels,
        CHUNK_SIZE  // Initial size matches our chunk size
    );

    if (!mDecoderInfo.fifo) {
        const std::string message = "Couldn't allocate FIFO.\n";
        handler.onError(error::Error::FileReadError, message);
        return false;
    }

    bool end_of_file = false;
    while (!end_of_file) {
        // Read packets until FIFO is full enough or file ends
        while (av_audio_fifo_size(mDecoderInfo.fifo) < CHUNK_SIZE) {
            if (av_read_frame(mDecoderInfo.format_ctx, packet) < 0) {
                end_of_file = true;
                break;
            }

            if (packet->stream_index != mDecoderInfo.stream_idx) {
                av_packet_unref(packet);
                continue;
            }

            ret = avcodec_send_packet(mDecoderInfo.decoder_ctx, packet);
            av_packet_unref(packet);

            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                const std::string message = "Couldn't decode packets.\n";
                handler.onError(error::Error::FileReadError, message);

                avformat_close_input(&mDecoderInfo.format_ctx);
                avcodec_parameters_free(&mDecoderInfo.decoder_params);
                av_frame_free(&frame);
                swr_free(&resampler);
                av_packet_free(&packet);
                av_audio_fifo_free(mDecoderInfo.fifo);
                return false;
            }

            // Get all available frames
            while ((ret = avcodec_receive_frame(mDecoderInfo.decoder_ctx, frame)) == 0) {
                AVFrame *resampled = av_frame_alloc();
                resampled->ch_layout = frame->ch_layout;
                resampled->format = AV_SAMPLE_FMT_FLTP;
                resampled->sample_rate = frame->sample_rate;

                ret = swr_convert_frame(resampler, resampled, frame);
                av_frame_unref(frame);

                if (ret < 0) {
                    const std::string message = "Couldn't resample audio frame.\n";
                    handler.onError(error::Error::FileReadError, message);

                    av_frame_free(&resampled);
                    avformat_close_input(&mDecoderInfo.format_ctx);
                    avcodec_parameters_free(&mDecoderInfo.decoder_params);
                    av_frame_free(&frame);
                    swr_free(&resampler);
                    av_packet_free(&packet);
                    av_audio_fifo_free(mDecoderInfo.fifo);
                    return false;
                }

                // Write to FIFO
                if (av_audio_fifo_write(mDecoderInfo.fifo, (void**)resampled->data, resampled->nb_samples) != resampled->nb_samples) {
                    const std::string message = "Couldn't write to FIFO.\n";
                    handler.onError(error::Error::FileReadError, message);
                    
                    av_frame_free(&resampled);
                    avformat_close_input(&mDecoderInfo.format_ctx);
                    avcodec_parameters_free(&mDecoderInfo.decoder_params);
                    av_frame_free(&frame);
                    swr_free(&resampler);
                    av_packet_free(&packet);
                    av_audio_fifo_free(mDecoderInfo.fifo);
                    return false;
                }

                av_frame_free(&resampled);
            }
        }

        // Read from FIFO and append to our temp buffers
        const int available_samples = av_audio_fifo_size(mDecoderInfo.fifo);
        if (available_samples > 0) {
            // Temporary buffers for each channel
            std::vector<float*> channel_buffers(mDecoderInfo.decoder_ctx->ch_layout.nb_channels);
            for (int ch = 0; ch < mDecoderInfo.decoder_ctx->ch_layout.nb_channels; ch++) {
                channel_buffers[ch] = new float[available_samples];
            }

            // Read from FIFO
            if (av_audio_fifo_read(mDecoderInfo.fifo, (void**)channel_buffers.data(), available_samples) < available_samples) {
                const std::string message = "Couldn't read from FIFO.\n";
                handler.onError(error::Error::FileReadError, message);

                avformat_close_input(&mDecoderInfo.format_ctx);
                avcodec_parameters_free(&mDecoderInfo.decoder_params);
                av_frame_free(&frame);
                swr_free(&resampler);
                av_packet_free(&packet);
                av_audio_fifo_free(mDecoderInfo.fifo);
                return false;
            }

            // Append to our temp buffers
            for (int ch = 0; ch < mDecoderInfo.decoder_ctx->ch_layout.nb_channels; ch++) {
                temp_buffers[ch].insert(temp_buffers[ch].end(), 
                    channel_buffers[ch], 
                    channel_buffers[ch] + available_samples);
                delete[] channel_buffers[ch];
            }

            mDecoderInfo.total_samples_per_chan += available_samples;
        }

        // Reset FIFO to read new data.
        av_audio_fifo_reset(mDecoderInfo.fifo);
    }

    // Move data to pAudio buffers
    for (int ch = 0; ch < mDecoderInfo.decoder_ctx->ch_layout.nb_channels; ch++) {
        pAudio->at(ch) = std::move(temp_buffers[ch]);
    }

    setAudioInfo(mDecoderInfo.decoder_ctx, mDecoderInfo.total_samples_per_chan);

    // Cleanup
    avformat_close_input(&mDecoderInfo.format_ctx);
    avcodec_parameters_free(&mDecoderInfo.decoder_params);
    avcodec_free_context(&mDecoderInfo.decoder_ctx);
    av_audio_fifo_free(mDecoderInfo.fifo);
    av_frame_free(&frame);
    swr_free(&resampler);
    av_packet_free(&packet);

    return true;
}

bool AJ::io::MP3_File::read(AJ::error::IErrorHandler &handler){
    //* 1. open file and find audio stream.
    if(!openFile(handler)){
        return false;
    }

    //* 2. initialize and open the decoder.
    if(!initDecoder(handler)){
        return false;
    }

    //* 3. start decoding and writing to pAudio buffers.
    if(!decode(handler)){
        return false;
    }

    return true;
}

bool AJ::io::MP3_File::initEncoder(AJ::error::IErrorHandler &handler){
    mEncoderInfo.encoder = avcodec_find_encoder(AV_CODEC_ID_MP3);
    
    if(!mEncoderInfo.encoder){
        const std::string message = "Couldn't find MP3 Encoder.\n";
        handler.onError(error::Error::FileWriteError, message);

        return false;   
    }

    mEncoderInfo.encoder_ctx = avcodec_alloc_context3(mEncoderInfo.encoder);


    if(!mEncoderInfo.encoder_ctx){
        const std::string message = "Couldn't allocate MP3 Encoder Context.\n";
        handler.onError(error::Error::FileWriteError, message);
        
        return false;   
    }


    mEncoderInfo.encoder_ctx->sample_rate = mWriteInfo.samplerate;
    mEncoderInfo.encoder_ctx->sample_fmt = AV_SAMPLE_FMT_S16P;

    if(mWriteInfo.channels == 1)
        mEncoderInfo.encoder_ctx->ch_layout = AV_CHANNEL_LAYOUT_MONO;

    if(mWriteInfo.channels == 2)
        mEncoderInfo.encoder_ctx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;


    if(avcodec_open2(mEncoderInfo.encoder_ctx, mEncoderInfo.encoder, NULL) < 0){
        const std::string message = "Couldn't open MP3 Encoder.\n";
        handler.onError(error::Error::FileWriteError, message);
        avcodec_free_context(&mEncoderInfo.encoder_ctx);
        return false;       
    }

    return true;
}

void AJ::io::MP3_File::allocateFrame(AVFrame *frame , int frame_size, AJ::error::IErrorHandler &handler){
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->nb_samples = frame_size;
    frame->sample_rate = mEncoderInfo.encoder_ctx->sample_rate;

    int ret = av_channel_layout_copy(&frame->ch_layout, &mEncoderInfo.encoder_ctx->ch_layout);
    
    if(ret != 0){
        const std::string message = "Couldn't copy channel layout for audio frame.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);

        return;  
    }

    ret = av_frame_get_buffer(frame, AV_INPUT_BUFFER_PADDING_SIZE);

    if(ret != 0){
        const std::string message = "Couldn't get buffer for audio frame.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);

        return;
    }


    ret = av_frame_make_writable(frame);

    if(ret != 0){
        const std::string message = "Couldn't make frame writable.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);

        return;
    }

}

void AJ::io::MP3_File::resampleFrameData(AVFrame *frame, AVFrame *resampled, SwrContext *resampler, AJ::error::IErrorHandler &handler){
    resampled->ch_layout = frame->ch_layout;
    resampled->ch_layout.nb_channels = frame->ch_layout.nb_channels;
    resampled->format = AV_SAMPLE_FMT_S16P; // uint16_t planar.
    resampled->sample_rate = frame->sample_rate;
    resampled->nb_samples = frame->nb_samples;

    int ret = av_frame_get_buffer(resampled, AV_INPUT_BUFFER_PADDING_SIZE);

    if(ret != 0){
        const std::string message = "Couldn't get buffer for resampled audio frame.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);
        av_frame_free(&resampled);
        swr_free(&resampler);

        return;
    }


    ret = av_frame_make_writable(resampled);

    if(ret != 0){
        const std::string message = "Couldn't make resampled frame writable.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);
        av_frame_free(&resampled);
        swr_free(&resampler);

        return;
    }

    ret = swr_convert_frame(resampler, resampled, frame);
    av_frame_unref(frame);

    if(ret != 0){
        const std::string message = "Couldn't resample data.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&resampled);

        return;  
    }
}

bool AJ::io::MP3_File::encode(AJ::error::IErrorHandler &handler){
    //* 1. Init packet 
    // packet holding encoded output.
    mEncoderInfo.packet = av_packet_alloc();

    if(!mEncoderInfo.packet){
        const std::string message = "Couldn't allocate audio packet.\n";
        handler.onError(error::Error::FileWriteError, message);

        return false;  
    }


    //* 2. init resampler
    // create a resampler to convert from float planar to uint16_t.
    SwrContext *resampler = swr_alloc();

    const AVChannelLayout *chan_layout = &mEncoderInfo.encoder_ctx->ch_layout;
    
    int ret = swr_alloc_set_opts2(&resampler, 
        chan_layout, // output channel layout.
        AV_SAMPLE_FMT_S16P, // output format (uint16_t planar)
        mEncoderInfo.encoder_ctx->sample_rate, // output samplerate.
        chan_layout, // input channel layout.
        AV_SAMPLE_FMT_FLTP, // input format (float planar).
        mEncoderInfo.encoder_ctx->sample_rate, // input sample rate.
        0, nullptr
    );

    if(ret != 0 || swr_init(resampler) != 0){
        const std::string message = "Couldn't initialize audio resampler.\n";
        handler.onError(error::Error::FileWriteError, message);
        swr_free(&resampler);
        return false;
    }

    //* 3. start encoding and writing data in chunks.
    /*
        * frame holding input raw audio (PCM Float).
        * frame size is usually 1152 so it must be encoded in chunks
        * data is gonna be written in chunks also. 
    */ 
    
    size_t frame_size = mEncoderInfo.encoder_ctx->frame_size;
    AVFrame *frame = av_frame_alloc();

    if(!frame){
        const std::string message = "Couldn't allocate audio frame.\n";
        handler.onError(error::Error::FileWriteError, message);
        swr_free(&resampler);

        return false;  
    }

    AVFrame *resampled = av_frame_alloc();

    if(!resampled){
        const std::string message = "Couldn't allocate resampled audio frame.\n";
        handler.onError(error::Error::FileWriteError, message);
        av_frame_free(&frame);
        swr_free(&resampler);

        return false;  
    }

    for(size_t offset = 0; offset < pAudio->at(0).size(); offset += frame_size){
        //* 1. set frame and copy chunk of data to it.
        int nb_samples = std::min(frame_size, pAudio->at(0).size() - offset);

        allocateFrame(frame, nb_samples, handler);

        if(!frame){
            swr_free(&resampler);
            return false;
        }

        
        for(short ch = 0; ch < frame->ch_layout.nb_channels; ch++){
            memcpy(frame->data[ch],
                pAudio->at(ch).data() + offset,
                nb_samples * sizeof(float)
            );
        }

        //* 2. resample to S16P.
        // the frame is unref in the resampleFrameData.
        resampleFrameData(frame, resampled, resampler, handler);

        if(!resampled){
            return false;
        }

        // send audio frame to the encoder context.
        int ret = avcodec_send_frame(mEncoderInfo.encoder_ctx, resampled);
        av_frame_unref(resampled);    

        if(ret != 0){
            const std::string message = "Couldn't send audio frame to the encoder context.\n";
            handler.onError(error::Error::FileWriteError, message);
            swr_free(&resampler);
            return false;  
        }

        //* 3. write the encoded chunk.
        if(!writeData(handler)){
            return false;
        }
    }
    
    av_frame_free(&frame);
    av_frame_free(&resampled);


    //* 4. Flush the encoder.
    if(avcodec_send_frame(mEncoderInfo.encoder_ctx, nullptr) != 0){
        const std::string message = "Couldn't send audio frame to the encoder context.\n";
        handler.onError(error::Error::FileWriteError, message);

        swr_free(&resampler);
        return false;
    }

    if(!writeData(handler)){
        swr_free(&resampler);
        return false;
    }

    swr_free(&resampler);

    return true;
}

bool AJ::io::MP3_File::writeData(AJ::error::IErrorHandler &handler){
    int ret = 0;
    // read all available output packets 
    while(ret >= 0){
        ret = avcodec_receive_packet(mEncoderInfo.encoder_ctx, mEncoderInfo.packet);
    
        /*
            * needs more packets,
            * that's normal just break this loop.
            * in the next iteration in encode function it will have the needed samples.
            * don't free the packet now just skip it.
        */
        if(ret == AVERROR(EAGAIN)){
            break;
        }

        // end of the file.
        else if(ret == AVERROR_EOF){
            // free memory.
            av_packet_free(&mEncoderInfo.packet);
            
            // succeed
            return true;
        } else if (ret < 0) { // error
            const std::string message = "Couldn't read the encoded data from the encoder.\n";
            handler.onError(error::Error::FileWriteError, message);

            av_packet_free(&mEncoderInfo.packet);
            return false;  
        }

        // there is no need to care about planar data the encoder will handle it.
        fwrite(mEncoderInfo.packet->data, 1, mEncoderInfo.packet->size, mEncoderInfo.file);
        av_packet_unref(mEncoderInfo.packet);
    }

    return true;
}

bool AJ::io::MP3_File::write(AJ::error::IErrorHandler &handler){

    //* 1. find MP3 encoder, alloc encoder context and open encoder.
    
    bool failed = !initEncoder(handler);

    if(failed){
        avcodec_free_context(&mEncoderInfo.encoder_ctx);
        return false;
    }

    //* 2. Open file.
    std::string fullPath = mWriteInfo.path + "/" + mWriteInfo.name + mWriteInfo.format;
    mEncoderInfo.file = fopen(fullPath.c_str(), "wb");

    if(!mEncoderInfo.file){
        const std::string message = "Couldn't open MP3 File in the targeted path: " 
            + mWriteInfo.path + "\n";
        handler.onError(error::Error::FileOpenError, message);

        avcodec_free_context(&mEncoderInfo.encoder_ctx);
        return false;  
    }

    
    //* 3. encoding and write data.
    
    failed = !encode(handler);
    
    if(mEncoderInfo.packet)
        av_packet_free(&mEncoderInfo.packet);
    
    // should be non-null here; extra check for safety.
    if(mEncoderInfo.encoder_ctx)
        avcodec_free_context(&mEncoderInfo.encoder_ctx);

    
    fclose(mEncoderInfo.file);
    
    if(failed){
        return false;
    }
    
    return true;
}
