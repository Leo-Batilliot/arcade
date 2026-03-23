# ── Embed snakemusic.ogg as a C++ byte array ──────────────────────────────────
set(_SNAKE_MUSIC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/snakemusic.ogg")
set(_SNAKE_MUSIC_GEN "${CMAKE_CURRENT_BINARY_DIR}/SnakeMusic.gen.cpp")
if(EXISTS "${_SNAKE_MUSIC_SRC}")
    file(READ "${_SNAKE_MUSIC_SRC}" _MUSIC_BYTES_HEX HEX)
    string(LENGTH "${_MUSIC_BYTES_HEX}" _MUSIC_HEX_LEN)
    math(EXPR _MUSIC_BYTE_LEN "${_MUSIC_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _MUSIC_BYTES_COMMA "${_MUSIC_BYTES_HEX}")
    file(WRITE "${_SNAKE_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char SNAKE_MUSIC_DATA[] = {${_MUSIC_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  SNAKE_MUSIC_SIZE   = ${_MUSIC_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade_snake] Embedded snakemusic.ogg (${_MUSIC_BYTE_LEN} bytes)")
else()
    file(WRITE "${_SNAKE_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char SNAKE_MUSIC_DATA[] = {0x00};\n"
        "extern const unsigned int  SNAKE_MUSIC_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(WARNING "[arcade_snake] games/snake/assets/snakemusic.ogg not found — audio disabled")
endif()

# ── Embed snake_head.png as a C++ byte array ──────────────────────────────────
set(_SNAKE_HEAD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/snake_head.png")
set(_SNAKE_HEAD_GEN "${CMAKE_CURRENT_BINARY_DIR}/SnakeHead.gen.cpp")
if(EXISTS "${_SNAKE_HEAD_SRC}")
    file(READ "${_SNAKE_HEAD_SRC}" _HEAD_BYTES_HEX HEX)
    string(LENGTH "${_HEAD_BYTES_HEX}" _HEAD_HEX_LEN)
    math(EXPR _HEAD_BYTE_LEN "${_HEAD_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _HEAD_BYTES_COMMA "${_HEAD_BYTES_HEX}")
    file(WRITE "${_SNAKE_HEAD_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char SNAKE_HEAD_DATA[] = {${_HEAD_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  SNAKE_HEAD_SIZE   = ${_HEAD_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade_snake] Embedded snake_head.png (${_HEAD_BYTE_LEN} bytes)")
else()
    file(WRITE "${_SNAKE_HEAD_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char SNAKE_HEAD_DATA[] = {0x00};\n"
        "extern const unsigned int  SNAKE_HEAD_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(WARNING "[arcade_snake] games/snake/assets/snake_head.png not found — head image disabled")
endif()

add_library(arcade_snake SHARED
    Snake.cpp
    "${_SNAKE_MUSIC_GEN}"
    "${_SNAKE_HEAD_GEN}"
)

target_include_directories(arcade_snake PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/games
    ${CMAKE_SOURCE_DIR}/graphics
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(arcade_snake PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)

set_target_properties(arcade_snake PROPERTIES
    PREFIX ""
    OUTPUT_NAME "arcade_snake"
)
