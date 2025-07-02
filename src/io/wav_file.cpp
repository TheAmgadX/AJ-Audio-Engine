#include <iostream>
#include <vector>
#include <cstdint> // for some data types like uint32_t
#include <fstream>

/* 
    used for writing bytes in the Write Function 
        take the file object, 
        void pointer to the data (data may be int or char[] for header fields),
        how many bytes will be written.
*/

// TODO: make the function checking for Endianness and handle writing on big or little Endianness
bool write_as_bytes(std::ofstream &file, const void *val, const size_t byte_size){
    return static_cast<bool>(
        file.write(
            (reinterpret_cast<const char*>(val)), byte_size
        )
    );
}