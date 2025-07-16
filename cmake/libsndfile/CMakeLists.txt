# Find libsndfile
find_library(SNDFILE_LIBRARY sndfile REQUIRED)
find_path(SNDFILE_INCLUDE_DIR sndfile.h REQUIRED)
include_directories(${SNDFILE_INCLUDE_DIR})