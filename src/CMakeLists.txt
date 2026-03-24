include(${CMAKE_SOURCE_DIR}/cmake/ArcadeEmbed.cmake)
arcade_embed_asset("assets/Losing.ogg")
arcade_embed_asset("assets/win.ogg")

# ── Embed win.ogg (stub with 0 bytes if file not yet present) ─────────────────
set(_WIN_OGG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/win.ogg")
set(_WIN_OGG_GEN "${CMAKE_CURRENT_BINARY_DIR}/WIN_OGG.gen.cpp")
if(EXISTS "${_WIN_OGG_SRC}")
    file(READ "${_WIN_OGG_SRC}" _WIN_BYTES_HEX HEX)
    string(LENGTH "${_WIN_BYTES_HEX}" _WIN_HEX_LEN)
    math(EXPR _WIN_BYTE_LEN "${_WIN_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _WIN_BYTES_COMMA "${_WIN_BYTES_HEX}")
    file(WRITE "${_WIN_OGG_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char WIN_OGG_DATA[] = {${_WIN_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  WIN_OGG_SIZE   = ${_WIN_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade] Embedded win.ogg (${_WIN_BYTE_LEN} bytes)")
else()
    file(WRITE "${_WIN_OGG_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char WIN_OGG_DATA[] = {0x00};\n"
        "extern const unsigned int  WIN_OGG_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade] src/assets/win.ogg not found — win sound disabled")
endif()

# ── Embed menumusic.ogg as a C++ byte array ───────────────────────────────────
set(_MENU_MUSIC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/menumusic.ogg")
set(_MENU_MUSIC_GEN "${CMAKE_CURRENT_BINARY_DIR}/MenuMusic.gen.cpp")
if(EXISTS "${_MENU_MUSIC_SRC}")
    file(READ "${_MENU_MUSIC_SRC}" _MUSIC_BYTES_HEX HEX)
    string(LENGTH "${_MUSIC_BYTES_HEX}" _MUSIC_HEX_LEN)
    math(EXPR _MUSIC_BYTE_LEN "${_MUSIC_HEX_LEN} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _MUSIC_BYTES_COMMA "${_MUSIC_BYTES_HEX}")
    file(WRITE "${_MENU_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char MENU_MUSIC_DATA[] = {${_MUSIC_BYTES_COMMA}0x00};\n"
        "extern const unsigned int  MENU_MUSIC_SIZE   = ${_MUSIC_BYTE_LEN}u;\n"
        "} // namespace Arcade\n")
    message(STATUS "[arcade] Embedded menumusic.ogg (${_MUSIC_BYTE_LEN} bytes)")
else()
    file(WRITE "${_MENU_MUSIC_GEN}"
        "// Auto-generated — do not edit.\n"
        "namespace Arcade {\n"
        "extern const unsigned char MENU_MUSIC_DATA[] = {0x00};\n"
        "extern const unsigned int  MENU_MUSIC_SIZE   = 0u;\n"
        "} // namespace Arcade\n")
    message(WARNING "[arcade] src/assets/menumusic.ogg not found — menu music disabled")
endif()

add_executable(arcade
    main.cpp
    core/CoreInit.cpp
    core/CoreMenu.cpp
    core/CoreGame.cpp
    core/CoreGameOver.cpp
    core/CoreVolume.cpp
    "${_MENU_MUSIC_GEN}"
    "${_WIN_OGG_GEN}"
    ${ARCADE_ASSET_SOURCES}
)

target_include_directories(arcade PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(arcade PRIVATE
    dl
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(arcade PRIVATE stdc++fs)
endif()

target_compile_options(arcade PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)

set_target_properties(arcade PROPERTIES
    OUTPUT_NAME "arcade"
)
