#pragma once
#include <cstdint>

namespace AJ{

constexpr int8_t kNumChannels = 2;
constexpr int16_t kBlockSize   = 2048;
constexpr int8_t kCombFilters = 4;
constexpr int8_t kAllPassFilters = 2;

// REVERB CONSTANTS
constexpr short REVERB_DELAY_MIN = 20;
constexpr int REVERB_DELAY_MAX = 1000;
constexpr short REVERB_MIX_MIN = -2.0;
constexpr short REVERB_MIX_MAX = 2.0;
constexpr short REVERB_WET_MIX = 0.3f;
constexpr short REVERB_DRY_MIX = 0.7f;
constexpr float REVERB_DELAY = 78.9f;
constexpr float REVERB_GAIN_MIN = 0.0f;
constexpr float REVERB_GAIN_MAX = 0.99f;
constexpr float REVERB_GAIN = 0.45f;
constexpr float COMB_FILTER_1_DELAY = -11.73f;
constexpr float COMB_FILTER_2_DELAY = 19.31f;
constexpr float COMB_FILTER_3_DELAY = -7.97f;


};
