## 🧠 Engine Overview

The `AJ_Engine` class is the **central interface** of the audio engine. It abstracts away file I/O, audio buffer management, DSP effect application, and undo handling — making it easy to build real-world audio applications.

### 🔧 Responsibilities

The engine is responsible for:

* **Loading** audio files (currently supports `.wav` via libsndfile and `.mp3` via FFmpeg)
* **Saving** audio files with metadata such as sample rate, channels, etc.
* **Applying DSP Effects** like Reverb, Echo, Gain, etc.
* **Supporting Functionalities** like mix, cut, insert.
* **Undo Support** (planned, not yet implemented)

### 🧪 Overloaded `applyEffect()` API

You can apply effects using three overloaded versions of `applyEffect()`:

| Target                                    | Description                               |
| ----------------------------------------- | ----------------------------------------- |
| `FloatBuffer`                             | Applies effect to a single-channel buffer |
| `std::shared_ptr<AudioFile>`              | Applies effect to all channels of a file  |
| `std::vector<std::shared_ptr<AudioFile>>` | Applies effect to a list of audio files   |

⚠️ **Note**:
Special operations like `cut`, `insert`, and `mix` are **not** part of `applyEffect()` and will be supported later via **dedicated APIs**.

⚠️ **Note**:
Undo system **exists in design** but is **not implemented** yet.

---

### 📁 File Locations

| File                             | Purpose                            |
| -------------------------------- | ---------------------------------- |
| `include/core/aj_audio_engine.h` | Public API declaration             |
| `src/core/aj_audio_engine.cc`    | Implementation of the engine logic |

---

### 🚀 Basic Example (`main.cc`)

The following example demonstrates how to use the engine to:

1. Load a stereo WAV file
2. Apply a Reverb effect to both channels
3. Save the processed result to disk

```cpp
#include <iostream>
#include "core/aj_audio_engine.h"

int main(){
    std::shared_ptr<AJ::AJ_Engine> engine = AJ::AJ_Engine::create();

    const std::string path = "/path/to/audio/violin.wav";
    const std::string ext = "wav";
    AJ::error::ConsoleErrorHandler handler;

    std::shared_ptr<AJ::io::AudioFile> audio = engine->loadAudio(path, handler, ext);
    if(!audio) return 0;

    auto params = std::make_shared<AJ::dsp::reverb::ReverbParams>();
    params->mDelayMS = 40.0f;
    params->mDryMix = 0.3f;
    params->mWetMix = 0.7f;
    params->mGain = 0.7f;
    params->mSamplerate = audio->mInfo.samplerate;
    params->mStart = 0;
    params->mEnd = (audio->mInfo.length / audio->mInfo.channels) - 1;

    // Apply to each channel
    for (int i = 0; i < audio->mInfo.channels; ++i) {
        if(!engine->applyEffect(audio->pAudio->at(i), AJ::Effect::reverb, params, handler)) {
            return 0;
        }
    }

    // Prepare write info
    AJ::AudioWriteInfo writeInfo;
    writeInfo.bitdepth = audio->mInfo.bitdepth;
    writeInfo.samplerate = audio->mInfo.samplerate;
    writeInfo.channels = audio->mInfo.channels;
    writeInfo.length = audio->mInfo.length;
    writeInfo.seekable = audio->mInfo.seekable;
    writeInfo.format = audio->mInfo.format;
    writeInfo.name = "reverbed_violin";
    writeInfo.path = "/path/to/output/";

    audio->setWriteInfo(writeInfo, handler);

    if(!engine->saveAudio(audio, handler)){
        return 0;
    }
}
```
---

## 🎚️ Supported Effects

The `Effect` enum located at **include/core/types.h** defines all DSP (Digital Signal Processing) operations currently supported by the `applyEffect()` methods:

```cpp
/**
 * @brief Enumerates the supported DSP (Digital Signal Processing) effects.
 *
 * This enum defines the list of audio effects that can be applied to audio buffers
 * using the applyEffect() functions. Each effect corresponds to a specific type of
 * signal processing operation.
 */
enum Effect {
    Distortion,     // Adds harmonic saturation or clipping
    echo,           // Adds delay-based repetitions
    reverb,         // Simulates space/room reflections
    fadeIn,         // Gradually increases volume from 0 to full
    fadeOut,        // Gradually decreases volume from full to 0
    gain,           // Amplifies or attenuates audio
    normalization,  // Normalizes signal to target level
    pitchShift,     // Alters pitch without changing speed
    reverse         // Plays audio in reverse (backwards)
};
```

Each effect corresponds to a signal transformation and can be applied to:

* ✅ A single audio channel (`Float &buffer`)
* ✅ A full audio file (multi-channel)
* ✅ A list of audio files.

These are passed to the `applyEffect()` function to specify which transformation to apply to the target audio data.

---
