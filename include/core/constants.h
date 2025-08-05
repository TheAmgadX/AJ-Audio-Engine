#pragma once
#include <cstdint>

namespace AJ{

/// @brief Number of audio channels (stereo = 2).
constexpr int8_t kNumChannels = 2;

/// @brief Block size used for audio processing (e.g., for delay buffers).
constexpr int16_t kBlockSize = 2048;

// -----------------------------
// Reverb Configuration Constants
// -----------------------------

/// @brief Number of comb filters used in the reverb effect.
constexpr int8_t kCombFilters = 4;

/// @brief Number of all-pass filters used in the reverb effect.
constexpr int8_t kAllPassFilters = 2;

/// @brief Minimum delay (in samples or ms, depending on usage) for the reverb effect.
constexpr short REVERB_DELAY_MIN = 20;

/// @brief Maximum delay (in samples or ms, depending on usage) for the reverb effect.
constexpr int REVERB_DELAY_MAX = 1000;

/// @brief Minimum allowed mix value for the reverb effect.
constexpr short REVERB_MIX_MIN = -2;

/// @brief Maximum allowed mix value for the reverb effect.
constexpr short REVERB_MIX_MAX = 2; 

/// @brief Default wet (effected) mix ratio for reverb.
constexpr float REVERB_WET_MIX = 0.3f;

/// @brief Default dry (original) mix ratio for reverb.
constexpr float REVERB_DRY_MIX = 0.7f;

/// @brief Default delay value used in reverb (in milliseconds or samples).
constexpr float REVERB_DELAY = 78.9f;

/// @brief Minimum gain (feedback amount) allowed for reverb filters.
constexpr float REVERB_GAIN_MIN = 0.0f;

/// @brief Maximum gain (feedback amount) allowed for reverb filters.
constexpr float REVERB_GAIN_MAX = 0.99f;

/// @brief Default gain value used in reverb filters.
constexpr float REVERB_GAIN = 0.45f;

// -----------------------------
// Specific Comb Filter Delays
// -----------------------------

/// @brief Delay time for the first comb filter.
constexpr float COMB_FILTER_1_DELAY = -11.73f;

/// @brief Delay time for the second comb filter.
constexpr float COMB_FILTER_2_DELAY = 19.31f;

/// @brief Delay time for the third comb filter.
constexpr float COMB_FILTER_3_DELAY = -7.97f;
};
