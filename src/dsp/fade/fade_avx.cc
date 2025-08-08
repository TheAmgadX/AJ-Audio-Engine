#include <algorithm>

#include "dsp/fade.h"
#include "core/types.h"
#include "core/error_handler.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
*/

bool AJ::dsp::Fade::fadeAVX(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->mEnd < mParams->mStart || mParams->mStart < 0 || mParams->mEnd >= buffer.size()){
        const std::string message = "invalid indexes for fade effect.\n";
        handler.onError(error::Error::InvalidEffectParameters, message);

        return false;
    }

    float currentGain;

    float gainDiff = mParams->highGain() - mParams->lowGain();
    sample_c totalSamples = mParams->mEnd - mParams->mStart + 1;
    double gainStep = gainDiff / totalSamples;

    if(mParams->mode() == FadeMode::In){
        currentGain = mParams->lowGain();
    } else {
        currentGain = mParams->highGain();
        gainStep *= -1;
    }

    /**
     * How it works:
     * Each sample gets a different gain based on its position.
     * - We use a "steps" vector to indicate how many gain steps ahead each element
     *   in the current block is from the starting gain.
     * - Adding these step offsets (multiplied by gainStep) to currentGain
     *   produces the gain vector for this block.
     * - We multiply the input samples by the gain vector and store the results.
     * - After processing a block, currentGain is increased by 8 steps for the next block.
     */

    const __m256 max_val = _mm256_set1_ps(1.0f);
    const __m256 min_val = _mm256_set1_ps(-1.0f);

    const __m256 gain_step = _mm256_set1_ps(gainStep);
    __m256 steps = _mm256_setr_ps(0, 1, 2, 3, 4, 5, 6, 7);

    sample_pos i;
    for(i = mParams->mStart; i + 7 <= mParams->mEnd; i += 8){
        // get the current gain per sample.
        __m256 gain = _mm256_add_ps(
            _mm256_set1_ps(currentGain),
            _mm256_mul_ps(steps, gain_step)
        );

        __m256 samples = _mm256_mul_ps( // multiply current samples by gain value
            _mm256_loadu_ps(&buffer[i]), // laod 8 current samples
            gain
        );

        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 8 samples in the buffer
        _mm256_storeu_ps(
            &buffer[i],
             _mm256_max_ps (
                _mm256_min_ps(samples, max_val),
                min_val
            )
        );

        currentGain += gainStep * 8; // 8 steps
    }

    // calculate the rest if there.
    for(i; i <= mParams->mEnd; ++i){
        buffer[i] = std::clamp(buffer[i] * currentGain, -1.0f, 1.0f);
    }

    return true;
}