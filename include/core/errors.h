#pragma once

namespace AJ::error {

/**
 * @brief Enumeration of error codes used throughout the audio engine.
 *
 * This enum provides standardized error values to represent different
 * failure states during file I/O, decoding, encoding, processing, etc.
 * 
 * these codes are used for consistent error reporting and handling across
 * the system.
 */
enum class Error {
    // Success (0)
    Success = 0,               ///< Operation completed successfully

    // File System Errors (100-199)
    FileNotFound = 100,        ///< Specified file could not be found
    InvalidFilePath,           ///< Path contains invalid characters or is malformed
    UnsupportedFileFormat,     ///< File format is not supported by the engine
    FileReadError,            ///< Failed to read data from file
    FileWriteError,           ///< Failed to write data to file
    FileOpenError,            ///< Failed to open file (permissions/locked/etc)
    FileClosingError,         ///< Error occurred while closing file
    DirectoryNotFound,        ///< Specified directory does not exist
    InsufficientPermissions,  ///< Lack of permissions for file operation

    // Audio Content Errors (200-299)
    EmptyAudioBuffer = 200,    ///< Audio buffer contains no samples
    InvalidSampleRate,         ///< Sample rate is not supported or invalid
    InvalidChannelCount,       ///< Channel count is not supported or invalid
    CorruptedAudioData,        ///< Audio data is corrupted or malformed
    InvalidAudioLength,        ///< Audio length is invalid or mismatched
    InvalidBitDepth,          ///< Bit depth is not supported or invalid
    BufferSizeMismatch,       ///< Buffer size does not match expected size
    BufferOverflow,           ///< Operation would exceed buffer capacity
    
    // DSP/Effect Errors (300-399)
    UnknownEffect = 300,       ///< Requested effect is not registered
    InvalidEffectParameters,   ///< Effect parameters are invalid or out of range
    DSPProcessingFailed,       ///< DSP processing operation failed
    InsufficientSampleData,    ///< Not enough samples for effect processing
    InvalidProcessingRange,    ///< Invalid start/end positions for processing
    EffectNotInitialized,     ///< Effect used before initialization
    
    // Engine/System Errors (400-499)
    EngineNotInitialized = 400, ///< Engine used before initialization
    OperationNotAllowed,        ///< Operation not allowed in current state
    UndoStackEmpty,             ///< No operations available to undo/redo
    ResourceAllocationFailed,   ///< Failed to allocate required resources
    InvalidConfiguration,       ///< Engine configuration is invalid
    
    // Internal Errors (500-599)
    InternalError = 500,        ///< Unexpected internal error occurred
    MemoryError,                ///< Memory-related operation failed
    StateError,                 ///< Invalid internal state detected
    UnhandledException,         ///< Unhandled exception occurred

    RecordingError = 600,
    InvalidBufferSize,
    RingBufferOverflow,
    NullBufferPtr,
    EmptyBufferQueue,
};

}
