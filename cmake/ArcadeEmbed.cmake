# arcade_embed_asset(<path>)
#
# Embeds any binary file (image, audio, …) as a C++ byte array.
#
# <path> — path to the asset, relative to CMAKE_CURRENT_SOURCE_DIR.
#
# The symbol names are derived automatically from the filename:
#   assets/snake_head.png  →  SNAKE_HEAD_PNG_DATA / SNAKE_HEAD_PNG_SIZE
#   assets/snakemusic.ogg  →  SNAKEMUSIC_OGG_DATA / SNAKEMUSIC_OGG_SIZE
#
# The generated .gen.cpp path is appended to ARCADE_ASSET_SOURCES.
# Add that variable to your add_library() call after all arcade_embed_asset() calls.
#
# Usage:
#   include(${CMAKE_SOURCE_DIR}/cmake/ArcadeEmbed.cmake)
#
#   arcade_embed_asset("assets/mymusic.ogg")
#   arcade_embed_asset("assets/mysprite.png")
#
#   add_library(arcade_mygame SHARED MyGame.cpp ${ARCADE_ASSET_SOURCES})
#
# In C++, declare the symbols with:
#   extern const unsigned char MYMUSIC_OGG_DATA[];
#   extern const unsigned int  MYMUSIC_OGG_SIZE;

macro(arcade_embed_asset ASSET_PATH)
    # ── Resolve full source path ───────────────────────────────────────────────
    if(IS_ABSOLUTE "${ASSET_PATH}")
        set(_AEA_SRC "${ASSET_PATH}")
    else()
        set(_AEA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_PATH}")
    endif()

    if(NOT EXISTS "${_AEA_SRC}")
        message(FATAL_ERROR "arcade_embed_asset: file not found: ${_AEA_SRC}")
    endif()

    # ── Derive symbol name from filename ──────────────────────────────────────
    # e.g. "snake_head.png" → "SNAKE_HEAD_PNG"
    get_filename_component(_AEA_FNAME "${_AEA_SRC}" NAME)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" _AEA_SYM "${_AEA_FNAME}")
    string(TOUPPER "${_AEA_SYM}" _AEA_SYM)

    # ── Output path ───────────────────────────────────────────────────────────
    set(_AEA_GEN "${CMAKE_CURRENT_BINARY_DIR}/${_AEA_SYM}.gen.cpp")

    # ── Read and convert ──────────────────────────────────────────────────────
    file(READ "${_AEA_SRC}" _AEA_HEX HEX)
    string(LENGTH "${_AEA_HEX}" _AEA_HEXLEN)
    math(EXPR _AEA_BYTELEN "${_AEA_HEXLEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _AEA_BYTES "${_AEA_HEX}")

    file(WRITE "${_AEA_GEN}"
        "// Auto-generated from ${_AEA_FNAME} — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char ${_AEA_SYM}_DATA[] = {${_AEA_BYTES}0x00};\n"
        "extern const unsigned int  ${_AEA_SYM}_SIZE   = ${_AEA_BYTELEN}u;\n"
        "} // namespace Arcade\n")

    message(STATUS "[${CMAKE_CURRENT_SOURCE_DIR}] Embedded ${_AEA_FNAME} → ${_AEA_SYM} (${_AEA_BYTELEN} bytes)")

    list(APPEND ARCADE_ASSET_SOURCES "${_AEA_GEN}")
endmacro()
