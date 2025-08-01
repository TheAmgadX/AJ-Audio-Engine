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
class MP3_File final : public AJ::io::AudioFile {
    // Struct to hold internal decoder state and FFmpeg objects
    struct AudioDecoder {
        int stream_idx;                             // Index of the audio stream in the media file
        AVFormatContext* format_ctx;                // Format context for demuxing
        AVCodecParameters* decoder_params;          // Codec parameters for the selected audio stream
        const AVCodec* decoder;                     // Decoder used to decode audio
        AVCodecContext* decoder_ctx;                // Decoder context for audio stream
        AVAudioFifo* fifo;                          // FIFO buffer holding decoded float PCM samples
        sample_c total_samples;                     // Total number of decoded audio samples

        AudioDecoder() {
            stream_idx = -1;
            format_ctx = nullptr;
            decoder_params = nullptr;
            decoder = nullptr;
            decoder_ctx = nullptr;
            fifo = nullptr;
            total_samples = 0;
        }
    };

    AudioDecoder mDecoderInfo;

    // ======== Internal Reading and Decoding helper methods ========

    /**
     * @brief Opens the input file and initializes the format context.
     *        Finds the audio stream and sets its index.
     *
     * @param handler Reference to the error handler for reporting issues.
     * @return true if successful, false otherwise.
     */
    bool openFile(AJ::error::IErrorHandler& handler);

    /**
     * @brief Finds and initializes the audio decoder for the stream.
     *        Only mono and stereo audio are supported.
     *
     * @param handler Reference to the error handler for reporting issues.
     * @return true if successful, false otherwise.
     */
    bool initDecoder(AJ::error::IErrorHandler& handler);

    /**
     * @brief Decodes audio packets into planar float PCM samples and pushes them into the FIFO buffer.
     *
     * @param handler Reference to the error handler for reporting issues.
     * @return true if decoding succeeds, false otherwise.
     */
    bool decode(AJ::error::IErrorHandler& handler);

    /**
     * @brief set the `mInfo` metadata for the decoded audio.
     *
     * @param decoder_ctx Pointer to the active decoder context.
     * @param total_samples Total number of samples decoded.
     */
    void setAudioInfo(AVCodecContext* decoder_ctx, sample_c& total_samples);

public:
    MP3_File() {
        mDecoderInfo = AudioDecoder();
    }

    /**
     * @brief Reads and decodes an MP3 file. Other formats may work if supported by FFmpeg,
     *        but only MP3 is well tested.
     *        it's also tested for FLAC.
     *        
     * @param handler Reference to the error handler for reporting issues.
     * @return true if successful, false otherwise.
     */
    bool read(AJ::error::IErrorHandler& handler) override;


    /// @brief 
    /// @param handler 
    /// @return 
    bool write(AJ::error::IErrorHandler &handler) override;
};
};