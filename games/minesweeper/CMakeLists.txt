# ── Embed minesweeper.ogg as a C++ byte array ─────────────────────────────────
set(_MINES_MUSIC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/minesweeper.ogg")
set(_MINES_MUSIC_GEN "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperMusic.gen.cpp")
if(EXISTS "${_MINES_MUSIC_SRC}")
    file(READ "${_MINES_MUSIC_SRC}" _MUSIC_BYTES_HEX HEX)
    string(LENGTH "${_MUSIC_BYTES_HEX}" _MUSIC_HEX_LEN)
    math(EXPR _MUSIC_BYTE_LEN "${_MUSIC_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _MUSIC_BYTES_COMMA "${_MUSIC_BYTES_HEX}")
    file(WRITE "${_MINES_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char MINES_MUSIC_DATA[] = {${_MUSIC_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  MINES_MUSIC_SIZE   = ${_MUSIC_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade_minesweeper] Embedded minesweeper.ogg (${_MUSIC_BYTE_LEN} bytes)")
else()
    file(WRITE "${_MINES_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char MINES_MUSIC_DATA[] = {0x00};\n"
        "extern const unsigned int  MINES_MUSIC_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(WARNING "[arcade_minesweeper] games/minesweeper/assets/minesweeper.ogg not found — audio disabled")
endif()

# ── Helper macro: embed a binary asset as a C++ byte array ────────────────────
macro(embed_asset SRC_PATH GEN_PATH ARRAY_NAME SIZE_NAME LABEL)
    if(EXISTS "${SRC_PATH}")
        file(READ "${SRC_PATH}" _HEX HEX)
        string(LENGTH "${_HEX}" _HEXLEN)
        math(EXPR _BYTELEN "${_HEXLEN} / 2")
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _COMMA "${_HEX}")
        file(WRITE "${GEN_PATH}"
            "// Auto-generated — do not edit.\n"
            "namespace Arcade {\n"
            "extern const unsigned char ${ARRAY_NAME}[] = {${_COMMA}0x00};\n"
            "extern const unsigned int  ${SIZE_NAME}   = ${_BYTELEN}u;\n"
            "} // namespace Arcade\n")
        message(STATUS "[arcade_minesweeper] Embedded ${LABEL} (${_BYTELEN} bytes)")
    else()
        file(WRITE "${GEN_PATH}"
            "namespace Arcade {\n"
            "extern const unsigned char ${ARRAY_NAME}[] = {0x00};\n"
            "extern const unsigned int  ${SIZE_NAME}   = 0u;\n"
            "} // namespace Arcade\n")
        message(WARNING "[arcade_minesweeper] ${SRC_PATH} not found")
    endif()
endmacro()

embed_asset("${CMAKE_CURRENT_SOURCE_DIR}/assets/flag.jpg"
            "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperFlag.gen.cpp"
            MINES_FLAG_DATA MINES_FLAG_SIZE "flag.jpg")

embed_asset("${CMAKE_CURRENT_SOURCE_DIR}/assets/bomb.png"
            "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperBomb.gen.cpp"
            MINES_BOMB_DATA MINES_BOMB_SIZE "bomb.png")

embed_asset("${CMAKE_CURRENT_SOURCE_DIR}/assets/unknown_cell.png"
            "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperUnknown.gen.cpp"
            MINES_UNKNOWN_DATA MINES_UNKNOWN_SIZE "unknown_cell.png")

add_library(arcade_minesweeper SHARED
    Minesweeper.cpp
    "${_MINES_MUSIC_GEN}"
    "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperFlag.gen.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperBomb.gen.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/MinesweeperUnknown.gen.cpp"
)

target_include_directories(arcade_minesweeper PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/games
    ${CMAKE_SOURCE_DIR}/graphics
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(arcade_minesweeper PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)

set_target_properties(arcade_minesweeper PROPERTIES
    PREFIX ""
    OUTPUT_NAME "arcade_minesweeper"
)
