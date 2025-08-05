#pragma once 

#include <iostream>
#include "errors.h"
#include <string>

namespace AJ::error {
/**
 * @brief Interface for custom error handling.
 * 
 * Implement this interface to define how your application or library
 * responds to error events. The error handling logic can be customized 
 * per use case (e.g., logging, throwing exceptions, GUI notifications, etc.).
 */
class IErrorHandler {
public:
    /**
     * @brief Called when an error occurs.
     * 
     * @param err The error code describing the type of failure.
     * @param errorMessage A detailed message providing context about the error.
     */
    virtual void onError(Error err, const std::string &errorMessage) = 0;

    IErrorHandler() = default;

    /// @brief Virtual destructor for safe inheritance.
    virtual ~IErrorHandler() = default;
};

/**
 * @brief Default implementation of IErrorHandler that outputs to console
 */
class ConsoleErrorHandler : public IErrorHandler {
public:
    void onError(Error err, const std::string &errorMessage) override {
        std::cerr << "\033[1;31m[AJ-Engine Error]\033[0m ";
        
        // Print category based on error code range with color coding
        if (static_cast<int>(err) >= 500) std::cerr << "\033[1;35m[Internal]\033[0m ";      // Magenta for internal
        else if (static_cast<int>(err) >= 400) std::cerr << "\033[1;33m[Engine]\033[0m ";   // Yellow for engine
        else if (static_cast<int>(err) >= 300) std::cerr << "\033[1;36m[DSP]\033[0m ";      // Cyan for DSP
        else if (static_cast<int>(err) >= 200) std::cerr << "\033[1;34m[Audio]\033[0m ";    // Blue for audio
        else if (static_cast<int>(err) >= 100) std::cerr << "\033[1;32m[File]\033[0m ";     // Green for file
        
        std::cerr << errorMessage << "\033[90m(Error Code: " << static_cast<int>(err) << ")\033[0m\n";
    }
};
}