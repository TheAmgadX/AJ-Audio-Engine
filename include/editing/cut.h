#pragma once
#include "core/types.h"
#include "core/errors.h"
#include "core/error_handler.h"
#include "file_io/audio_file.h"

namespace AJ::editing::cut {

/**
 * @class Cut
 * @brief Provides functionality to remove a contiguous range of samples from an audio buffer.
 *
 * The Cut class operates directly on a buffer of floating-point samples.
 * It supports cutting any single continuous range, where the end index is inclusive.
 * The original order of remaining samples is preserved.
 *
 * Typical usage:
 * @code
 * AJ::editing::cut::Cut cutter;
 * cutter.setRange(100, 200, errorHandler); // Remove samples 100 to 200 inclusive
 * cutter.process(buffer, errorHandler);
 * @endcode
 */
class Cut {
private:
    /**
     * @brief Start index of the cut range (inclusive).
     * Initialized to -1 to indicate "unset".
     */
    sample_c mStart;

    /**
     * @brief End index of the cut range (inclusive).
     * Initialized to -1 to indicate "unset".
     */
    sample_c mEnd;

public:
    /**
     * @brief Constructs a new Cut instance with an uninitialized range.
     */
    Cut() : mStart(-1), mEnd(-1) {}

    /**
     * @brief Sets the cut range.
     *
     * @param start The starting sample index (inclusive).
     * @param end   The ending sample index (inclusive).
     * @param handler Reference to an error handler for reporting errors.
     * 
     * @return true if the range was set successfully, false otherwise.
     *
     * @note The range is considered valid if:
     *       - start >= 0
     *       - end >= start
     */
    bool setRange(sample_c start, sample_c end, AJ::error::IErrorHandler& handler) {
        if (start > end || start < 0) {
            const std::string message = "Invalid cut range. Start index must be >= 0 and <= end index.";
            handler.onError(error::Error::InvalidProcessingRange, message);

            return false;
        }
        mStart = start;
        mEnd = end;
        return true;
    }

    /**
     * @brief Processes the cut operation on the provided audio buffer.
     *
     * @param file Shared pointer to the AudioFile object whose audio data will be modified in-place.
     *             The cut will be applied to all channels, and file metadata such as length will
     *             be updated accordingly.
     * @param handler Reference to an error handler for reporting errors.
     * @return true if the operation succeeded, false otherwise.
     *
     * @note The end index is inclusive. The buffer is modified in-place.
     * @note You must call setRange() with valid start and end indices before invoking this method.
     */
    bool process(std::shared_ptr<AJ::io::AudioFile> file, AJ::error::IErrorHandler& handler);
};

} // namespace AJ::editing::cut
