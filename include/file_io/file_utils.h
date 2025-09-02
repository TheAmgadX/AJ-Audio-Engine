#pragma once
#include "core/constants.h"
#include "core/types.h"

#include <string>

namespace AJ::utils {

class FileUtils {
public:

    static bool make_directory(std::string& directory);

    /**
     * @brief Verifies that the given path exists and is a directory.
     * 
     * @param path Directory path to check.
     * @return true if the directory exists; false otherwise.
     */
    static bool valid_directory(std::string& directory);

    /**
     * @brief Trims leading and trailing whitespace from a file name.
     * 
     * @param name Reference to the file name string.
     * @return true if the resulting name is non-empty; false otherwise.
     */
    static bool trim_file_name(std::string &name);

    /**
     * @brief Verifies that a given path points to a valid file.
     * 
     * @param path File path to check.
     * 
     * @return true if the file exists and is regular; false otherwise.
     */
    static bool file_exists(std::string& file_path);

    /**
     * @brief Checks whether the provided file extension is supported.
     * 
     * Only `.wav` and `.mp3` are currently supported.
     * 
     * @param ext The file extension (case-insensitive).
     * @return true if the extension is supported; false otherwise.
     */
    static bool available_file_extension(std::string ext);

    /**
     * @brief Extract the file extension from the full path of the file.
     * 
     * @param path Reference to the full path which is directory + file name.
     * 
     * @return the extension if found in the file path; empty string otherwise.
     */
    static std::string get_file_extension(std::string& path);

    static std::string generate_file_name(FileStreamingTypes type, std::string& extension);
};

};