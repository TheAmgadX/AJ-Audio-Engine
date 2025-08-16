# Common source files (without main or test)
set(COMMON_SOURCES
    src/file_io/wav_file.cc
    src/file_io/mp3_file.cc
    src/file_io/audio_file.cc

    src/dsp/echo/echo.cc
    src/dsp/gain/gain.cc

    src/dsp/reverb/all_pass_filter.cc
    src/dsp/reverb/comb_filter.cc
    src/dsp/reverb/reverb.cc
    
    src/dsp/fade/fade.cc
    src/dsp/normalization/normalization.cc
    src/dsp/normalization/gain.cc

    src/dsp/distortion/distortion.cc

    src/dsp/reverse/reverse.cc

    src/editing/cut.cc
    src/editing/insert.cc
)

