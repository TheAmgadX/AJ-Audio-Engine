#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

#include "file_io/wav_file.h"
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open


AJ::BitDepth_t AJ::io::WAV_File::get_bit_depth(const SF_INFO &info){
    int subtype = info.format & SF_FORMAT_SUBMASK;

    switch(subtype) {
        case SF_FORMAT_PCM_S8:    
            return BitDepth_t::int_8;

        case SF_FORMAT_PCM_16:    
            return BitDepth_t::int_16;

        case SF_FORMAT_PCM_24:    
            return BitDepth_t::int_24;

        case SF_FORMAT_PCM_32:    
            return BitDepth_t::int_32;

        case SF_FORMAT_FLOAT:     
            return BitDepth_t::float_32; // float is 32-bit float samples

        case SF_FORMAT_DOUBLE:    
            return BitDepth_t::float_64; // double precision float samples

        default:                  
            return Not_Supported; // Unknown / unsupported
    }
}

bool AJ::io::WAV_File::read_mono_data(SNDFILE *file) {
    size_t number_of_blocks = std::ceil(static_cast<float>(mInfo.length) / kBlockSize);

    (*pAudio)[0].resize(number_of_blocks);

    sample_c samples_read_total = 0;

    for (size_t block = 0; block < number_of_blocks; ++block) {
        sample_c samples_remaining = mInfo.length - samples_read_total;

        sample_c samples_to_read = std::min<sample_c>(kBlockSize, samples_remaining);

        (*pAudio)[0][block].resize(samples_to_read);

        float* samplesBlock = (*pAudio)[0][block].data();

        sample_c samples_read = sf_read_float(file, samplesBlock, samples_to_read);

        if (samples_read == 0) {
            std::cerr << "Problem while reading file's samples.\n";
            return false;
        }

        if (samples_read != samples_to_read) {
            (*pAudio)[0][block].resize(samples_read);
        }

        samples_read_total += samples_read;
    }

    if (sf_close(file) != 0) {
        std::cerr << "Error closing file\n";
        return false;
    }

    return true;
}

bool AJ::io::WAV_File::read_stereo_data(SNDFILE *file){
    Float file_samples;
    file_samples.resize(mInfo.length);

    sample_c samplesCount =  sf_read_float(file, file_samples.data(), mInfo.length);

    if (samplesCount % 2 != 0) {
        std::cerr << "Unexpected stereo sample count.\n";
        return false;
    }

    if(mInfo.length != samplesCount){
        std::cerr << "Failed to read the samples\n";
        return false;
    }

    // fill the data into blocks buffers
    size_t number_of_blocks = std::ceil((static_cast<float>(mInfo.length) / 2) / kBlockSize);
  
    (*pAudio)[0].resize(number_of_blocks);
    (*pAudio)[1].resize(number_of_blocks);
 
    sample_c sample = 0;
    size_t i = 0;
    for(size_t block = 0; block < number_of_blocks; ++block){
        i = 0;
        while(i < kBlockSize && sample < samplesCount){
            (* pAudio)[0][block].push_back(file_samples[sample]);
            (* pAudio)[1][block].push_back(file_samples[sample + 1]);

            ++i;
            sample += 2;
        }
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

void AJ::io::WAV_File::set_file_info(SF_INFO &info){
    info.channels = mWriteInfo.channels;
    info.frames = mWriteInfo.length / mWriteInfo.channels;
    info.seekable = mWriteInfo.seekable;
    info.samplerate = mWriteInfo.samplerate;
    
    // TODO: after implementing the Upsampling and Downsampling resample the audio data here.
    if (mWriteInfo.samplerate != mInfo.samplerate){

    }
}

bool AJ::io::WAV_File::write_samples_mono(SNDFILE *file){
    for(int block = 0; block < (*pAudio)[0].size(); ++block){
        int size = (*pAudio)[0][block].size();
        sample_c samples = sf_write_float(file, (*pAudio)[0][block].data(), size);
        if(samples != size){
            std::cerr << "Error: faild to write audio samples to file " << mWriteInfo.path 
            << "/" << mWriteInfo.name << "\n";
            return false;
        }
    }

    if (sf_close(file) != 0){
        std::cerr << "Error closing file\n";
        return false;
    } // sf_close returns 0 in successs}

    return true;
}

bool AJ::io::WAV_File::write_samples_stereo(SNDFILE *file){
    Float samples;
    
    // copy audio samples to samples buffer
    for(size_t block = 0; block < (*pAudio)[0].size(); ++block){
        int block_size = (*pAudio)[0][block].size();
        for(size_t i = 0; i < block_size; ++i){            
            samples.push_back((*pAudio)[0][block][i]);
            samples.push_back((*pAudio)[1][block][i]);
        }
    }

    sample_c written_samples = sf_write_float(file, samples.data(), samples.size());

    if(written_samples != samples.size()){
        std::cerr << "Error: faild to write audio samples to file " << mWriteInfo.path 
        << "/" << mWriteInfo.name << "\n";
        return false;
    }

    if (sf_close(file) != 0){
        std::cerr << "Error closing file\n";
        return false;
    } // sf_close returns 0 in successs}

    return true;
}

bool AJ::io::WAV_File::write(){
    SF_INFO info = {};

    set_file_info(info);
    
    // set format in info struct
    switch (mWriteInfo.bitdepth){
        case BitDepth_t::int_8:
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8;
            break;
        case BitDepth_t::int_16:
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            break;

        case BitDepth_t::int_24:
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;
            break;

        case BitDepth_t::int_32:
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
            break;

        case BitDepth_t::float_32:
            info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            break;

        case BitDepth_t::float_64:
            info.format = SF_FORMAT_WAV | SF_FORMAT_DOUBLE;
            break;

        default:
            std::cerr << "Not supported bit depth.\n";
            return false;
    }

    std::string fullPath = mWriteInfo.path + "/" + mWriteInfo.name;
    SNDFILE *file = sf_open(fullPath.c_str(), SFM_WRITE, &info);

    if(!file){
        std::cerr << "Couldn't create file at: " << mWriteInfo.path 
        << "/" << mWriteInfo.name << "\n";

        return false;
    }

    if(info.channels == 1){
        return write_samples_mono(file);
    } else if(info.channels == 2){
        return write_samples_stereo(file);
    } else {
        std::cerr << "Not supported channels number. \n";

        if (sf_close(file) != 0){
            std::cerr << "Error closing file\n";
            return false;
        } // sf_close returns 0 in successs}

        return false;
    }
}

