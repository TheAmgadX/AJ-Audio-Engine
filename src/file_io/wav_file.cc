#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

#include "include/file_io/wav_file.h"
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open

uint8_t AJ::io::WAV_File::get_bit_depth(const SF_INFO &info){
    int subtype = info.format & SF_FORMAT_SUBMASK;

    switch(subtype) {
        case SF_FORMAT_PCM_S8:    
            return 8;

        case SF_FORMAT_PCM_16:    
            return 16;

        case SF_FORMAT_PCM_24:    
            return 24;

        case SF_FORMAT_PCM_32:    
            return 32;

        case SF_FORMAT_FLOAT:     
            return 32; // float is 32-bit float samples

        case SF_FORMAT_DOUBLE:    
            return 64; // double precision float samples

        default:                  
            return -1; // Unknown / unsupported
    }

    return -1;
}

bool AJ::io::WAV_File::read_mono_data(SNDFILE *file){
    size_t number_of_blocks = std::ceil(static_cast<float>(mInfo.length) / kBlockSize);
   
    (*pAudio)[0].resize(number_of_blocks);
    
    for(size_t block = 0; block < number_of_blocks; ++block){

        float *samplesBlock = (*pAudio)[0][block].data();
        sample_c samples =  sf_read_float(file, samplesBlock, kBlockSize);

        if(samples == 0){
            std::cerr << "Problem while reading file's samples.\n";
            return false;
        }
        if(samples < kBlockSize){
            // fill the rest with 0.
            std::fill((*pAudio)[0][block].begin() + samples,
                (*pAudio)[0][block].end(), 0);
        }
    }
    

    if (sf_close(file) != 0){
        std::cerr << "Error closing file\n";
        return false;
    } // sf_close returns 0 in successs}

    return true;
}

bool AJ::io::WAV_File::read_stereo_data(SNDFILE *file){
    Float file_samples;
    file_samples.resize(mInfo.length);

    sample_c samplesCount =  sf_read_float(file, file_samples.data(), mInfo.length);

    if(mInfo.length != samplesCount){
        std::cerr << "Failed to read the samples\n";
        return false;
    }

    // fill the data into blocks buffers
    size_t number_of_blocks = std::ceil(static_cast<float>(mInfo.length) / kBlockSize);
  
    (*pAudio)[0].resize(number_of_blocks);
    (*pAudio)[1].resize(number_of_blocks);

    sample_c sample = 0;
    size_t i = 0;
    for(size_t block = 0; block < number_of_blocks; ++block){
        i = 0;
        while(i < kBlockSize && sample < samplesCount){
            (* pAudio)[0][block][i] = file_samples[sample]; // left chan
            (* pAudio)[1][block][i] = file_samples[sample + 1]; // right chan

            ++i;
            sample += 2;
        }
    }

    // fill the rest of the last block with 0
    if(i < kBlockSize){
        // left chan
        std::fill((*pAudio)[0][number_of_blocks - 1].begin() + i,
            (*pAudio)[0][number_of_blocks - 1].end(), 0);

        // right chan
        std::fill((*pAudio)[1][number_of_blocks - 1].begin() + i,
            (*pAudio)[1][number_of_blocks - 1].end(), 0);
    }

    if (sf_close(file) != 0){
        std::cerr << "Error closing file\n";
        return false;
    } // sf_close returns 0 in successs

    return true;
}

bool AJ::io::WAV_File::read() {
    SF_INFO sfInfo = {};

    SNDFILE *snd_file = sf_open(mFilePath.c_str(), SFM_READ, &sfInfo);

    if(!snd_file){
        std::cerr << "Failed to open " << mFileName
            << " " << sf_strerror(nullptr) << "\n";

        return false;
    }
    
    // check file format
    if((sfInfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV){
        std::cerr << "Unsupported file format.\n";
        
        if (sf_close(snd_file) != 0){
            std::cerr << "Error closing file\n";
        } // sf_close returns 0 in successs

        return false;
    } else {
        mInfo.format = ".wav";
    }

    mInfo.channels = sfInfo.channels;
    mInfo.length = sfInfo.frames * sfInfo.channels;
    mInfo.samplerate = sfInfo.samplerate;
    mInfo.seekable = sfInfo.seekable;
    mInfo.bitdepth = get_bit_depth(sfInfo);

    // TODO: support more than two channels in the future.
    if(kNumChannels < mInfo.channels){
        std::cerr << "AJ-Engine only support mono and stereo audio\n";
        
        if (sf_close(snd_file) != 0){
            std::cerr << "Error closing file\n";
        } // sf_close returns 0 in successs

        return false;
    }

    if(mInfo.channels == 1){
        return read_mono_data(snd_file);
    } else {
        return read_stereo_data(snd_file);
    }
}   