#pragma once

#include "core/types.h"
#include "core/errors.h"
#include "core/error_handler.h"
#include "file_io/audio_file.h"

namespace AJ::editing::insert {

/**
 * @class Insert
 * @brief Provides functionality to insert audio samples into an existing AudioFile.
 *
 * The Insert class supports three specialized insertion modes:
 * - **pushFront**: Insert audio at the beginning of the file.
 * - **pushBack**: Insert audio at the end of the file.
 * - **insert**: Insert audio at a specific index in the middle of the file.
 *
 * @warning The provided AudioSamples are moved into the file.
 *          This means the contents of the input @p audio will be lost after calling
 *          process() If you need to preserve the original samples, make a copy before passing them in.
 *
 * This design avoids unnecessary copies to improve performance when editing large audio buffers.
 */
class Insert {
    sample_c mInsertAt; ///< The sample index at which insertion should occur. -1 if not set.

    /**
     * @brief Appends audio to the end of the file.
     *
     * @param file Shared pointer to the target AudioFile to modify.
     * @param audio The audio samples to append. **This parameter is consumed (moved).**
     * 
     * @return true if the operation succeeded, false otherwise.
     */
    bool pushBack(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio);

    /**
     * @brief Inserts audio into the middle of the file at the index set by setInsertAt().
     *
     * @param file Shared pointer to the target AudioFile to modify.
     * @param audio The audio samples to insert. **This parameter is consumed (moved).**
     * 
     * @return true if the operation succeeded, false otherwise.
     */
    bool insert(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio);

    /**
     * @brief Inserts audio at the beginning of the file.
     *
     * @param file Shared pointer to the target AudioFile to modify.
     * @param audio The audio samples to prepend. **This parameter is consumed (moved).**
     * 
     * @return true if the operation succeeded, false otherwise.
     */
    bool pushFront(std::shared_ptr<AJ::io::AudioFile> file, AudioSamples audio);

public:

    /**
     * @brief Constructs a new Insert processor with no insertion index set.
     */
    Insert() : mInsertAt(-1) {}

    /**
     * @brief Sets the sample index at which to insert audio.
     *
     * @param index The index (in samples) where insertion should occur.
     * @param handler Reference to an error handler for reporting invalid indices.
     * 
     * @return true if the index is valid (>= 0), false otherwise.
     *
     * @note If an invalid index (< 0) is provided, an error is reported and the index remains unchanged.
     * @note To insert at the **beginning** of the file, set @p index = 0.
     * @note To insert at the **end** of the file, set @p index = file->pAudio->at(0).size() 
     *       (i.e., the current size of the target audio buffers).
     */

    bool setInsertAt(sample_c index, AJ::error::IErrorHandler& handler) {
        if(index < 0){
            const std::string message =
                "Invalid insert at index. index must be >= 0.";
            handler.onError(error::Error::InvalidProcessingRange, message);
            return false;
        }

        mInsertAt = index;
        return true;
    }

    /**
     * @brief Inserts the provided audio samples into the target AudioFile.
     *
     * Depending on the current insertion index:
     * - If index == 0 → pushFront() is called.
     * - If index == audio buffer size (file->pAudio->at(0).size()) → pushBack() is called.
     * - Otherwise → insert() is called.
     *
     * @param file Shared pointer to the AudioFile whose audio data will be modified in-place.
     *             The insert operation applies to all channels, and file metadata such as
     *             length will be updated accordingly.
     * @param audio The audio samples to insert. **This parameter is consumed (moved).**
     * @param handler Reference to an error handler for reporting errors.
     * 
     * @return true if the operation succeeded, false otherwise.
     *
     * @warning After calling this method, the contents of @p audio are no longer valid.
     *          Copy the samples first if you need to retain them.
     *
     * @note You must call setInsertAt() with a valid index before invoking this method.
     */
    bool process(std::shared_ptr<AJ::io::AudioFile> file,
                 AudioSamples audio,
                 AJ::error::IErrorHandler& handler);
};

} // namespace AJ::editing::insert
