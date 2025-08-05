## 🎛️ Audio Processing

The audio processing system in AJ is built around modular and extensible **effects**, which can be applied to audio data using the core `AJ_Engine` class. All processing is done on **float-based samples** (`Float`) to ensure high precision and flexibility.

---

## ⚙️ Effects System

All audio effects implement the `AJ::dsp::Effect` interface and must:

* Implement `process(Float&, IErrorHandler&)` for actual audio manipulation.
* Implement `setParams(std::shared_ptr<EffectParams>, IErrorHandler&)` for parameter configuration.

Each effect has a corresponding subclass of `EffectParams` to hold the parameters required for that effect (e.g., `ReverbParams` for `Reverb`).

All audio processing effects live in:

📁 `include/dsp/`
📁 `src/dsp/`

---

## 🧬 Parameters

`EffectParams` is the base class used to define the **start** and **end** sample positions. Each effect has its own subclass of `EffectParams` which holds custom parameters specific to that effect.

---

## 🌊 Reverb Effect

`Reverb` is a concrete implementation of the `Effect` interface. It uses:

* **Comb Filters** to simulate early reflections
* **All-Pass Filters** to simulate late reflections (echo tail smoothing)

It supports adjustable parameters:

* `DelayMS`
* `Gain`
* `DryMix`
* `WetMix`
* `Samplerate`

It processes only the region `[mStart, mEnd]` specified in `ReverbParams`.

---

## 🎚️ Supported Effects

The `Effect` enum defines all DSP (Digital Signal Processing) operations currently supported:

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

---

## 📐 Class Diagram (Mermaid)

```mermaid
classDiagram
    class AJ::dsp::Effect {
        <<interface>>
        +process(buffer, handler)
        +setParams(params, handler)
    }

    class AJ::dsp::EffectParams {
        +sample_pos mStart
        +sample_pos mEnd
        <<abstract>>
    }

    class AJ::dsp::reverb::ReverbParams {
        +float mDelayMS
        +float mWetMix
        +float mDryMix
        +int mSamplerate
        +float mGain
    }

    class AJ::dsp::reverb::Reverb {
        -CombFilters mCombFilters
        -AllPassFilters mAllPassFilters
        -shared_ptr<ReverbParams> mParams
        +setDelay(delayMS)
        +setDryMix(mix)
        +setWetMix(mix)
        +setGain(gain)
        +setSamplerate(samplerate)
        +setRange(start, end)
        +setParams(params, handler)
        +process(buffer, handler)
    }

    AJ::dsp::Effect <|-- AJ::dsp::reverb::Reverb
    AJ::dsp::EffectParams <|-- AJ::dsp::reverb::ReverbParams
    AJ::dsp::reverb::Reverb o-- AJ::dsp::reverb::ReverbParams
```

---

## 🗂️ Directory Layout

```
include/
└── dsp/
    ├── effect.h           # Base Effect interface
    └── reverb/
        ├── reverb.h       # Reverb effect class
        ├── all_pass_filter.h
        └── comb_filter.h

src/
└── dsp/
    ├── reverb/
        └── reverb.cc     # Reverb processing logic
```
---
