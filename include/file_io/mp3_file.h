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
// TODO: support resampling via the resampler already used in the encode function.

/**
 * @class MP3_File
 * @brief A class for handling MP3 file input/output using FFmpeg.
 *
 * This class is a concrete implementation of the abstract AudioFile interface,
 * designed to read and write MP3 audio files.
 *
 * It uses FFmpeg libraries internally to decode and encode MP3 streams,
 * and stores the decoded data in memory for offline processing.
 *
 * Features:
 * - Offline reading: the full MP3 file is decoded and loaded into memory.
 * - Supports reading per-channel float PCM buffers.
 * - MP3 writing via libavcodec, using encoding parameters derived from the
 *   provided audio metadata.
 *
 *
 * @see AJ::io::AudioFile
 */
class MP3_File final : public AJ::io::AudioFile {
    /// @brief Struct to hold internal decoder state and FFmpeg objects
    struct AudioDecoder {
        int stream_idx;                             // Index of the audio stream in the media file
        AVFormatContext* format_ctx;                // Format context for demuxing
        AVCodecParameters* decoder_params;          // Codec parameters for the selected audio stream
        const AVCodec* decoder;                     // Decoder used to decode audio
        AVCodecContext* decoder_ctx;                // Decoder context for audio stream
        AVAudioFifo* fifo;                          // FIFO buffer holding decoded float PCM samples
        sample_c total_samples_per_chan;                     // Total number of decoded audio samples

        AudioDecoder() {
            stream_idx = -1;
            format_ctx = nullptr;
            decoder_params = nullptr;
            decoder = nullptr;
            decoder_ctx = nullptr;
            fifo = nullptr;
            total_samples_per_chan = 0;
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
     * @param total_samples_per_chan Total number of samples decoded.
     */
    void setAudioInfo(AVCodecContext* decoder_ctx, sample_c& total_samples_per_chan);

    /// @brief Struct to hold internal encoder state and FFmpeg objects.
    struct AudioEncoder{
        const AVCodec *encoder;
        AVCodecContext *encoder_ctx;
        FILE *file;
        AVPacket *packet;
        
        AudioEncoder(){
            encoder = nullptr;
            encoder_ctx = nullptr;
            file = nullptr;
            packet = nullptr;
        }
    };

    AudioEncoder mEncoderInfo;

    // ======== Internal Writing and Encoding helper methods ========

    /**
     * @brief Initializes the MP3 encoder and configures the encoder context
     *        to use AV_SAMPLE_FMT_S16P (int16_t planar). Opens the encoder.
     *        Only mono and stereo channels are supported.
     * 
     * @param handler Reference to the error handler for reporting issues.
     * @return true on success, false on failure or if channels are unsupported.
     */
    bool initEncoder(AJ::error::IErrorHandler &handler);

    /**
     * @brief Allocates an AVFrame and sets its parameters to hold float planar audio.
     *        Copies settings from the encoder context, sets nb_samples to frame_size,
     *        allocates the buffer, and ensures it's writable.
     *        On failure, the frame is freed.
     * 
     * @param frame Pointer to the frame to be initialized.
     * @param frame_size Number of samples per channel.
     * @param handler Reference to the error handler for reporting issues.
     */
    void allocateFrame(AVFrame *frame, int frame_size, AJ::error::IErrorHandler &handler);

    /**
     * @brief Resamples the input frame (float planar) to S16P (int16_t planar) format.
     *        Uses the provided SwrContext and writes to the resampled frame.
     *        Makes the resampled frame writable, and unreferences the input frame.
     *        On failure, frees the resampler, resampled frame, and input frame.
     * 
     * @param frame Pointer to the input float frame.
     * @param resampled Pointer to the output resampled frame.
     * @param resampler Pointer to the resampling context.
     * @param handler Reference to the error handler for reporting issues.
     */
    void resampleFrameData(AVFrame *frame, AVFrame *resampled, SwrContext *resampler, AJ::error::IErrorHandler &handler);

    /**
     * @brief Writes encoded packets to the output file.
     * 
     * @param handler Reference to the error handler for reporting issues.
     * @return true on success, false on failure.
     */
    bool writeData(AJ::error::IErrorHandler &handler);

    /**
     * @brief Encodes audio samples and writes them to file via writeData().
     *        Internally initializes and manages the frame, resampled frame,
     *        and resampler context. These are automatically cleaned up.
     *        The caller must free the packet, encoder context, and close the file.
     * 
     * @param handler Reference to the error handler for reporting issues.
     * @return true on success, false otherwise.
     */
    bool encode(AJ::error::IErrorHandler &handler);


public:
    MP3_File() {
        mDecoderInfo = AudioDecoder();
        mEncoderInfo = AudioEncoder();
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

    // TODO: Optimize write function — currently performs slowly.
    /**
     * @brief Encodes and writes audio data to an MP3 file.
     *
     * @param handler Reference to the error handler for reporting issues.
     * @return true on success, false on failure.
     */
    bool write(AJ::error::IErrorHandler& handler) override;
};

};