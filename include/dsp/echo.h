#pragma once
#include "effect.h"
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"

namespace AJ::dsp {
/**
 * @brief Parameter container for the Echo effect.
 * 
 * Inherits from `EffectParams`. Stores parameters such as decay amount and delay in samples,
 *  as well as optional processing start and end ranges.
 *
 * @see AJ::dsp::EffectParams
*/   
class EchoParams : public EffectParams {
public:
    /// @brief The decay factor applied to the delayed signal.
    float mDecay;

    /// @brief The delay in number of samples.
    sample_c mDelaySamples;

    /// @brief Destructor.
    ~EchoParams() override = default;

    /// @brief Default constructor. Initializes the processing range to -1 (unspecified).
    EchoParams();

    /**
     * @brief Range constructor.
     * @param start Start sample position for applying the effect.
     * @param end End sample position for applying the effect.
    */
    EchoParams(sample_pos start, sample_pos end);
};

/** 
 * @brief Echo effect processor with architecture-specific implementations.
 *
 * Supports naive and SIMD-based echo effect processing (SSE, AVX).
 * Uses `EchoParams` for configuration. 
*/
class Echo : public AJ::dsp::Effect {
private:

    /**
     * @brief Naive implementation of the echo effect (no SIMD).
     * @param buffer The audio buffer to process in-place.
     * @param handler Error handler for reporting processing errors.
     * @return true if processing was successful.
    */
    bool echoNaive(Float &buffer, AJ::error::IErrorHandler &handler);
    /**
     * @brief SSE SIMD implementation of the echo effect.
     */
    bool echoSIMD_SSE(Float &buffer, AJ::error::IErrorHandler &handler);
    /**
     * @brief AVX SIMD implementation of the echo effect.
     */
    bool echoSIMD_AVX(Float &buffer, AJ::error::IErrorHandler &handler);

    /**
     * @brief Calculates a new sample value by adding delayed signal with decay.
     * @param in The input buffer.
     * @param sample_idx The current sample index.
     * @param echo_idx The delayed sample index.
     * @param handler Error handler.
     * @return The new sample value. 
     */
    sample_t calculate_new_sample_with_echo(Float &in, sample_pos sample_idx, sample_pos echo_idx, AJ::error::IErrorHandler &handler);
    /**
     * @brief Parameters for the echo effect.
     */
    std::shared_ptr<EchoParams> mParams;

public:
    Echo();

    /**
     * @brief Apply the echo effect to a buffer.
     * @param buffer Audio buffer to process.
     * @param handler Error handler.
     * @return true if successful, false otherwise.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Set effect parameters from a shared EffectParams object.
     *        Will downcast to EchoParams.
     * @param params Shared pointer to EffectParams.
     * @param handler Error handler.
     * @return true if parameters were valid and applied.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Get current decay value.
     * @return The decay factor.
     */
    decay_t GetDecay();
    /**
     * @brief Set the decay factor.
     * @param decay The new decay value.
     */
    void SetDecay(decay_t decay);

    /**
     * @brief Get current delay in samples.
     * @return The delay in samples.
    */
    sample_c GetDelaySampels();

    /**
     * @brief Set the delay using seconds and sample rate.
     * @param delayInSeconds Delay time in seconds.
     * @param sampleRate The sample rate of the audio.
    */
    void SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate);

    /**
     * @brief Restrict effect processing to a range in the buffer.
     * @param start Starting sample index.
     * @param end Ending sample index.
     */
    void setRange(sample_pos start, sample_pos end);
};

};