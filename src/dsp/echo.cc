#include<iostream>

#include "echo.h"
#include "types.h"


void AJ::dsp::Echo::process(){

}

AJ::sample_t AJ::dsp::Echo::CalculateNewSampleWithEcho(Float &in, sample_pos sample_idx, sample_pos echo_idx){
    sample_t newSample = in[sample_idx]  + in[sample_idx - _mDelaySamples];
    // TODO: Make sure it's in valid range.
    return newSample;
}

void AJ::dsp::Echo::SetDelaySamples(decay_t delayInSeconds, sample_c sampleRate){
    _mDelaySamples = sampleRate * delayInSeconds;
}

void AJ::dsp::Echo::EchoNaive(Float &in, Float &out, sample_pos start, sample_pos end){
    /*
        ?Echo Effect Equation:
            Echo_Sample = in[i - _mDelaySamples]
            out[i] = in[i] + Echo_Sample

        ! we must avoid writing at the input buffer to avoid speculative samples problem.
    */

    // TODO: there is a faster way to copy that using built in functions search for it and implement it
    //* copy the first samples
    for(sample_pos i = start; i < _mDelaySamples; ++i){
        out[i] = in[i];
    }

    for(sample_pos i = start + _mDelaySamples; i < end; ++i){
        sample_t sample = CalculateNewSampleWithEcho(in, i, i - _mDelaySamples);
        out[i] = sample;
    }
}

void AJ::dsp::Echo::EchoSIMD_8(Float &in, Float &out, sample_pos start, sample_pos end){

}

void AJ::dsp::Echo::EchoSIMD_16(Float &in, Float &out, sample_pos start, sample_pos end){

}

void AJ::dsp::Echo::EchoSIMD_32(Float &in, Float &out, sample_pos start, sample_pos end){

}