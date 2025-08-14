#pragma once
#include <memory>

#include "effect.h"
#include "core/types.h"
#include "core/effect_params.h"
#include "core/error_handler.h"
#include "core/errors.h"

namespace AJ::dsp::reverse {

/**
 * @brief Reversal selection parameters.
 *
 * @details
 * Defines the inclusive [mStart, mEnd] sample range to reverse.
 *
 * @invariant
 * - 0 <= mStart <= mEnd
 * - The range [mStart, mEnd] must lie within the buffer being processed.
 */
struct Params{
    sample_c mStart; ///< Start index (inclusive), in samples per channel.
    sample_c mEnd;   ///< End index (inclusive), in samples per channel.
};

/**
 * @class ReverseParams
 * @brief Validated parameter object for the Reverse effect.
 *
 * @details
 * This class wraps validated parameters for the Reverse effect.
 * Use the factory @ref create to construct; direct construction is restricted.
 * Validation errors are reported via @ref AJ::error::IErrorHandler and result in a nullptr return.
 *
 * @par Validation
 * - @c mStart and @c mEnd define an inclusive range.
 * - @c mStart <= @c mEnd.
 * - The caller is responsible for ensuring the specified range fits within the buffer used at processing time.
 * - The `process()` function in the Reverse class will verify the range boundaries.
 */
class ReverseParams : public EffectParams {
    struct PrivateTag{};
public:
    /**
     * @brief Factory: build and validate ReverseParams from a plain @ref Params.
     *
     * @param params   Input parameters (inclusive range).
     * @param handler  Error handler for reporting validation failures.
     * 
     * @return Shared pointer to a valid ReverseParams on success, or nullptr on failure.
     */
    static std::shared_ptr<ReverseParams> create(Params &params, AJ::error::IErrorHandler &handler);

    /// @brief Private constructor (use @ref create).
    ReverseParams(PrivateTag){
        setStart(-1);
        setEnd(-1);
    }
};

/**
 * @class Reverse
 * @brief In-place reversal effect over a selected sample range.
 *
 * @details
 * Reverses only the selected region [start, end] of the buffer; samples outside
 * the selection remain unchanged. The operation is performed in-place.
 *
 * @par Usage
 * 1) Build @ref ReverseParams via @ref ReverseParams::create.
 * 2) @ref setParams to apply them.
 * 3) Call @ref process on each channel buffer to reverse the region.
 */
class Reverse : public AJ::dsp::Effect {
    std::shared_ptr<ReverseParams> mParams;
public:
    /**
     * @brief Set validated parameters for processing.
     *
     * @param params  Shared pointer to EffectParams (must be ReverseParams).
     * @param handler Error handler for bad downcasts or invalid params.
     * 
     * @return true on success; false if params is null or of the wrong type.
     */
    bool setParams(std::shared_ptr<EffectParams> params, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Reverse the selected range of the given buffer.
     *
     * @param buffer  Audio buffer (planar) to modify in place.
     * @param handler Error handler.
     * 
     * @return true on success; false if parameters are missing or invalid.
     *
     * @pre @ref setParams has been called with a valid @ref ReverseParams.
     * @post Samples in [start, end] are reversed; others unchanged.
     */
    bool process(Float &buffer, AJ::error::IErrorHandler &handler) override;

    /**
     * @brief Override the selection range (inclusive).
     *
     * @param start Starting sample index (inclusive).
     * @param end   Ending sample index (inclusive).
     *
     * @note This helper updates internal params; it does not validate bounds
     * against the specific @p buffer. Ensure correctness before calling @ref process.
     */
    void setRange(sample_pos start, sample_pos end);
};

} // namespace AJ::dsp::reverse