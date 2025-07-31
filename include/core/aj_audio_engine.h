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
public:
    /**
     * @brief Creates a new instance of the AJ_Engine
     * @return std::shared_ptr<AJ_Engine> Smart pointer to the newly created engine instance
     */
    static std::shared_ptr<AJ_Engine> create();

    // TODO: add the undo work after processing.
    /**
     * @brief Applies a DSP (Digital Signal Processing) effect to a segment of an audio buffer.
     *
     * The effect is applied to the range [mStart, mEnd] as specified in the provided params object.
     * Available effects are defined in core/types.h under the Effect enum.
     * 
     * @param buffer Reference to the audio buffer to process
     * @param effect The effect to apply
     * @param params Parameters for the effect
     * @param handler Error handler for reporting processing issues
     * 
     * @return bool true if processing succeeded, false if an error occurred
     */
    bool applyEffect(Float &buffer, const Effect &effect, 
        std::shared_ptr<dsp::EffectParams> params, error::IErrorHandler &handler);

    /**
     * @brief Loads an audio file into memory.
     *
     * Supported formats include WAV and MP3. Returns a polymorphic AudioFile object
     * representing the loaded file.
     * 
     * @param path Path to the audio file
     * @param ext Expected file extension
     * @param handler Error handler for reporting loading issues
     * 
     * @return std::shared_ptr<io::AudioFile> Smart pointer to the loaded audio file,
     *         or nullptr if loading failed
     */
    std::shared_ptr<io::AudioFile> loadAudio(const std::string &path,
        const std::string &ext, error::IErrorHandler &handler);
    
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



    // Engine Members
    std::stack<AJ::undo::State> mStates;
    AJ::undo::UndoSystem mUndo;

};
}
