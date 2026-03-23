# ── Audio backend check ───────────────────────────────────────────────────────
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PULSEAUDIO QUIET libpulse)
    pkg_check_modules(ALSA       QUIET alsa)
    if(NOT PULSEAUDIO_FOUND AND NOT ALSA_FOUND)
        message(WARNING
            "[arcade_sdl2] libpulse-dev / libasound2-dev not found — "
            "SDL2 audio will be silent (dummy driver). "
            "Install one then rebuild from scratch: "
            "sudo apt install libpulse-dev")
    endif()
endif()

# ── Always fetch SDL2 / SDL2_ttf / SDL2_mixer as static ───────────────────────
set(CMAKE_POSITION_INDEPENDENT_CODE ON  CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS               OFF CACHE BOOL "" FORCE)

set(_SAVED_LIB_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(_SAVED_ARC_DIR "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fetchcontent_libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fetchcontent_libs")

# SDL2
message(STATUS "[arcade_sdl2] Fetching SDL2 2.30.3 (static) ...")
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.3 GIT_SHALLOW TRUE)
set(SDL_SHARED     OFF CACHE BOOL "" FORCE)
set(SDL_STATIC     ON  CACHE BOOL "" FORCE)
set(SDL_STATIC_PIC ON  CACHE BOOL "" FORCE)
set(SDL_ALSA       OFF CACHE BOOL "" FORCE)
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
FetchContent_MakeAvailable(SDL2)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

# SDL2_ttf (vendored freetype by default in 2.22.0)
message(STATUS "[arcade_sdl2] Fetching SDL2_ttf 2.22.0 (static) ...")
FetchContent_Declare(SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-2.22.0 GIT_SHALLOW TRUE)
set(SDL2TTF_INSTALL   OFF CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ  OFF CACHE BOOL "" FORCE)
set(SDL2_DIR "${sdl2_BINARY_DIR}" CACHE PATH "" FORCE)
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
FetchContent_MakeAvailable(SDL2_ttf)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

# SDL2_mixer (stb_vorbis bundled, no external audio codec deps)
message(STATUS "[arcade_sdl2] Fetching SDL2_mixer 2.8.0 (static) ...")
FetchContent_Declare(SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-2.8.0 GIT_SHALLOW TRUE)
set(SDL2MIXER_INSTALL  OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD      OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVPACK  OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VORBIS   STB CACHE STRING "" FORCE)
set(SDL2_DIR "${sdl2_BINARY_DIR}" CACHE PATH "" FORCE)
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
FetchContent_MakeAvailable(SDL2_mixer)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_SAVED_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_SAVED_ARC_DIR}")

# Restore shared libs for games / other targets processed after this
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

# ── Download stb_image.h for image support ────────────────────────────────────
set(_STB_IMAGE_H "${CMAKE_CURRENT_BINARY_DIR}/stb_image.h")
if(NOT EXISTS "${_STB_IMAGE_H}")
    message(STATUS "[arcade_sdl2] Downloading stb_image.h ...")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        "${_STB_IMAGE_H}"
        STATUS _stb_dl_status)
    list(GET _stb_dl_status 0 _stb_dl_code)
    if(NOT _stb_dl_code EQUAL 0)
        message(WARNING "[arcade_sdl2] stb_image.h download failed — image support disabled")
        set(_STB_IMAGE_H "")
    endif()
endif()

# ── Embed Inter.ttf ───────────────────────────────────────────────────────────
set(_ARCADE_FONT_SRC "${CMAKE_SOURCE_DIR}/graphics/assets/Inter.ttf")
set(_ARCADE_FONT_GEN "${CMAKE_CURRENT_BINARY_DIR}/InterFont.gen.cpp")
if(EXISTS "${_ARCADE_FONT_SRC}")
    file(READ "${_ARCADE_FONT_SRC}" _FONT_BYTES_HEX HEX)
    string(LENGTH "${_FONT_BYTES_HEX}" _FONT_HEX_LEN)
    math(EXPR _FONT_BYTE_LEN "${_FONT_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _FONT_BYTES_COMMA "${_FONT_BYTES_HEX}")
    file(WRITE "${_ARCADE_FONT_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char INTER_FONT_DATA[] = {${_FONT_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  INTER_FONT_SIZE   = ${_FONT_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
else()
    file(WRITE "${_ARCADE_FONT_GEN}"
        "namespace Arcade {\n"
        "extern const unsigned char INTER_FONT_DATA[] = {0x00};\n"
        "extern const unsigned int  INTER_FONT_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(WARNING "[arcade_sdl2] graphics/assets/Inter.ttf not found — text rendering disabled")
endif()

# ── Target ────────────────────────────────────────────────────────────────────
if(_STB_IMAGE_H AND EXISTS "${_STB_IMAGE_H}")
    add_library(arcade_sdl2 SHARED
        SDL2Display.cpp
        SDL2Image.cpp
        "${_ARCADE_FONT_GEN}")
    target_include_directories(arcade_sdl2 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    target_compile_definitions(arcade_sdl2 PRIVATE ARCADE_SDL2_IMAGES=1)
else()
    add_library(arcade_sdl2 SHARED SDL2Display.cpp "${_ARCADE_FONT_GEN}")
endif()

target_include_directories(arcade_sdl2 PRIVATE
    ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/graphics ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(arcade_sdl2 PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)

# Link SDL2 static targets
if(TARGET SDL2-static)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2-static)
    if(sdl2_SOURCE_DIR)
        target_include_directories(arcade_sdl2 PRIVATE "${sdl2_SOURCE_DIR}/include")
    endif()
elseif(TARGET SDL2::SDL2-static)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2::SDL2-static)
else()
    message(FATAL_ERROR "[arcade_sdl2] SDL2 static target not found after FetchContent")
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2_ttf::SDL2_ttf)
elseif(TARGET SDL2_ttf)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2_ttf)
    if(sdl2_ttf_SOURCE_DIR)
        target_include_directories(arcade_sdl2 PRIVATE "${sdl2_ttf_SOURCE_DIR}")
    endif()
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2_mixer::SDL2_mixer)
elseif(TARGET SDL2_mixer)
    target_link_libraries(arcade_sdl2 PRIVATE SDL2_mixer)
    if(sdl2_mixer_SOURCE_DIR)
        target_include_directories(arcade_sdl2 PRIVATE "${sdl2_mixer_SOURCE_DIR}/include")
    endif()
endif()

set_target_properties(arcade_sdl2 PROPERTIES
    PREFIX "" OUTPUT_NAME "arcade_sdl2")
