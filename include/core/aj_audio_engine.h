#pragma once
#include <stack>

#include <memory>
#include <string>

#include "file_io/audio_file.h"
#include "dsp/reverb/reverb.h"
#include "dsp/echo.h"
#include "dsp/gain.h"

#include "core/error_handler.h"
#include "core/effect_params.h"


#include "undo_system/state.h"
#include "undo_system/undo.h"

using Audio = std::shared_ptr<AJ::io::AudioFile>;

namespace AJ {

// TODO: cut, insert and mixing will have special APIs not like the applyEffect API
class AJ_Engine {

    // TODO: this stack may be moved to the UndoSystem class.
    /**
     * @brief Stack of saved undo states.
     *
     * Each state represents a snapshot of the system before a change was made.
     * Used to implement undo functionality by restoring previous states.
     */
    std::stack<AJ::undo::State> mStates;

    /**
     * @brief The undo system controller.
     *
     * Manages state registration, undo operations, and history limits.
     * Encapsulates the core logic for the undo functionality.
     */
    AJ::undo::UndoSystem mUndo;

    // TODO: this flag may be moved to the UndoSystem class.
    /**
     * @brief Flag indicating whether undo functionality is supported.
     *
     * When set to true, this object participates in the undo/redo system.
     * Used by components that maintain reversible state changes.
     */
    bool mUndoSupportEnabled;
public:
    /**
     * @brief Default constructor initializing that enables the undo system.
     */
    AJ_Engine(){
        mUndoSupportEnabled = true;
    }

    /**
     * @brief Creates a new instance of the AJ_Engine
     * @return std::shared_ptr<AJ_Engine> Smart pointer to the newly created engine instance
     */
    static std::shared_ptr<AJ_Engine> create();

    // TODO: add the undo functionality after processing.
    /**
     * @brief Applies a DSP effect to a single-channel audio buffer segment.
     *
     * The effect is applied to the range [mStart, mEnd], inclusive, as specified in the provided parameters.
     * Supported effects are defined in core/types.h under the Effect enum.
     *
     * @param buffer Reference to the single-channel audio buffer to process.
     * @param effect The effect to apply.
     * @param params Parameters for the effect (polymorphic based on the effect type).
     *               Use the appropriate subclass of EffectParams for the desired effect.
     *               Refer to the examples for how to construct effect-specific parameters.
     * @param handler Error handler for reporting processing issues.
     * 
     * @return true if processing succeeded, false otherwise.
     */
    bool applyEffect(Float &buffer, const Effect &effect, 
        std::shared_ptr<dsp::EffectParams> params, error::IErrorHandler &handler);

    // TODO: Support multi-threading per channel.
    // TODO: Add undo functionality after processing.

    /**
     * @brief Applies a DSP effect to all audio channels of a single audio file.
     *
     * The effect is applied to the range [mStart, mEnd], inclusive, on each channel buffer,
     * using the provided effect parameters.
     *
     * Supported effects are defined in core/types.h under the Effect enum.
     *
     * @param audio Shared pointer to the audio file to process.
     * @param effect The effect to apply.
     * @param params Parameters for the effect (polymorphic based on the effect type).
     *               Use the appropriate subclass of EffectParams for the desired effect.
     *               Refer to the examples for how to construct effect-specific parameters.
     * @param handler Error handler for reporting processing issues.
     * 
     * @return true if processing succeeded, false otherwise.
     */

    bool applyEffect(std::shared_ptr<AJ::io::AudioFile> audio, const Effect &effect, 
        std::shared_ptr<dsp::EffectParams> params, error::IErrorHandler &handler);

    // TODO: Support multi-threading per file.
    // TODO: Add undo functionality after processing.
    /**
     * @brief Applies a DSP effect to all audio channels of multiple audio files.
     *
     * The effect is applied to the range [mStart, mEnd], inclusive, on each channel
     * of each audio file in the provided list.
     *
     * Supported effects are defined in core/types.h under the Effect enum.
     *
     * @param audioFiles Vector of shared pointers to audio files to process.
     * @param effect The effect to apply.
     * @param params Parameters for the effect (polymorphic based on the effect type).
     *               Use the appropriate subclass of EffectParams for the desired effect.
     *               Refer to the examples for how to construct effect-specific parameters.     
     *  @param handler Error handler for reporting processing issues.
     * 
     * @return true if processing succeeded for all files, false otherwise.
     */
    bool applyEffect(std::vector<std::shared_ptr<AJ::io::AudioFile>> audioFiles, const Effect &effect, 
        std::shared_ptr<dsp::EffectParams> params, error::IErrorHandler &handler);

    /**
     * @brief Loads an audio file into memory.
     *
     * Supported formats include WAV and MP3. Returns a polymorphic AudioFile object
     * representing the loaded file.
     * 
     * @param path Full path to the audio file
     * @param handler Error handler for reporting loading issues
     * @param ext Expected file extensiion 
     * if not known don't pass it.
     * 
     * @return std::shared_ptr<io::AudioFile> Smart pointer to the loaded audio file,
     *         or nullptr if loading failed
     */
    std::shared_ptr<io::AudioFile> loadAudio(const std::string &path, error::IErrorHandler &handler,
        std::string ext = "");
    
    /**
     * @brief Saves an audio file to disk using its internal format.
     *
     * The file is saved in the same format it was originally loaded as.
     * For example, WAV_File objects are saved as WAV files, and MP3_File
     * objects are saved as MP3 files.
     *
     * Before calling this function, you must configure the audio object by
     * setting its write information via the `setWriteInfo` method. This method
     * requires an `AudioWriteInfo&` object specifying write settings and an
     * `IErrorHandler&` for reporting any configuration issues.
     * 
     * @param audio Smart pointer to the audio file to save
     * @param handler Error handler for reporting saving issues
     * 
     * @return bool true if save succeeded, false if an error occurred
     */
    bool saveAudio(std::shared_ptr<io::AudioFile> audio, error::IErrorHandler &handler);

    /**
     * @brief Enables or disables support for the undo system.
     * 
     * This flag determines whether the object participates in
     * the undo/redo operation stack.
     *
     * @param enabled Set to true to enable undo support; false to disable.
     */
    void setUndoSupportEnabled(bool enabled) {
        mUndoSupportEnabled = enabled;
    }

    /**
     * @brief Checks whether undo support is enabled for this object.
     * 
     * @return true if undo support is enabled, false otherwise.
     */
    bool isUndoSupportEnabled() const {
        return mUndoSupportEnabled;
    }

};
}
