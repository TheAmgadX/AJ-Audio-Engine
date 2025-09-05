# --- PortAudio via pkg-config ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Optional: create a tidy imported target
add_library(portaudio::portaudio INTERFACE IMPORTED)

set_target_properties(portaudio::portaudio PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES     "${PORTAUDIO_LINK_LIBRARIES}"
)