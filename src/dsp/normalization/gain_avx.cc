#include <iostream>

#include "dsp/normalization.h"
#include "core/types.h"
#include "core/error_handler.h"
#include "core/errors.h"

#include <immintrin.h> 
/* 
    - AVX (Advanced Vector Extensions) - 256-bit operations (8 floats)
*/

bool AJ::dsp::normalization::Normalization::gainAVX(Float &buffer, AJ::error::IErrorHandler &handler){
    // check valid indexes ranges
    if(mParams->End() < mParams->Start() || mParams->Start() < 0 || 
        mParams->Start() >= buffer.size() || mParams->End() >= buffer.size()){
        const std::string message = "invalid indexes for gain effect.";
        handler.onError(error::Error::InvalidEffectParameters, message);
        return false;
    }

    const __m256 gain = _mm256_set1_ps(mParams->Gain());

    sample_pos i;
    for(i = mParams->Start(); i + 7 <= mParams->End(); i += 8){
        __m256 samples = _mm256_mul_ps( // multiply current samples by gain value
            _mm256_loadu_ps(&buffer[i]), // laod 8 current samples
            gain
        );

        // clamping the calculated samples to make sure it's in valid range [-1.0, 1.0]
        // store the new 8 samples in the buffer
        _mm256_storeu_ps(
            &buffer[i],
            samples
        );
    }

    // calculate the rest if there.
    for(i; i <= mParams->End(); ++i){
        buffer[i] *= mParams->Gain();
    }

    return true;
}


