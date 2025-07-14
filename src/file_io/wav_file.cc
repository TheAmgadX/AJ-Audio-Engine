#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

#include "file_io/wav_file.h"
#include <core/types.h>
#include <sndfile.h> // docs: http://www.mega-nerd.com/libsndfile/api.html#open


bool AJ::io::WAV_File::close_file(SNDFILE *file, bool return_val){
    if (sf_close(file) != 0){
        std::cerr << "Error closing file\n";
        return false;
    } // sf_close returns 0 in success

    return return_val;
}

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
    (*pAudio)[0].resize(mInfo.length);

    sample_c samples = sf_read_float(file, (*pAudio)[0].data(), mInfo.length);

    if (samples == 0 || samples != mInfo.length) {
        std::cerr << "Problem while reading file's samples.\n";
        
        return close_file(file, false);  
    }

    return close_file(file, true);   
}

bool AJ::io::WAV_File::read_stereo_data(SNDFILE *file){
    if(mInfo.length < 0){
        std::cerr << "invalid file lenght.\n";
        return close_file(file, false);   
    }
    Float file_samples;
    file_samples.resize(mInfo.length);

    sample_c samplesCount =  sf_read_float(file, file_samples.data(), mInfo.length);

    if (samplesCount % 2 != 0) {
        std::cerr << "Unexpected stereo sample count.\n";
        return close_file(file, false);   
    }

    if(mInfo.length != samplesCount){
        std::cerr << "Failed to read the samples\n";
        return close_file(file, false);   
    }
   
   
    sample_c chan_samples = mInfo.length / 2;
    (*pAudio)[0].resize(chan_samples);
    (*pAudio)[1].resize(chan_samples);

    size_t sample = 0;
    for(size_t i = 0; i < chan_samples; ++i){
        (*pAudio)[0][i] = file_samples[sample]; // left chan
        (*pAudio)[1][i] = file_samples[sample + 1]; // right chan
        sample += 2;
    }

    return close_file(file, true);   
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
        
        return close_file(snd_file, false);   
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
        
        return close_file(snd_file, false);   
    }

    if(mInfo.channels == 1){
        return read_mono_data(snd_file);
    } else {
        return read_stereo_data(snd_file);
    }
}   

// TODO: after implementing the Upsampling and Downsampling resample the audio data here.
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
    sample_c samples = sf_write_float(file, (*pAudio)[0].data(), mInfo.length);

    if(samples != mInfo.length){
        std::cerr << "Error: faild to write audio samples to file " << mWriteInfo.path 
        << "/" << mWriteInfo.name << "\n";
        return close_file(file, false);
    }

    return close_file(file, true);
}

bool AJ::io::WAV_File::write_samples_stereo(SNDFILE *file){
    Float audio_samples;

    audio_samples.resize(mInfo.length);

    int i = 0;
    for(sample_pos sample = 0; sample < mInfo.length; sample += 2){
        audio_samples[sample] = (*pAudio)[0][i];
        audio_samples[sample + 1] = (*pAudio)[1][i];
        i++;
    }

    sample_c samples = sf_write_float(file, audio_samples.data(), mInfo.length);
    
    if(samples != mInfo.length){
        std::cerr << "Error: faild to write audio samples to file " << mWriteInfo.path 
        << "/" << mWriteInfo.name << "\n";
        return close_file(file, false);
    }

    return close_file(file, true);
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
        return close_file(file, false);
    }
}

