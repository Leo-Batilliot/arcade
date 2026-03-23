# ── Always fetch SFML 3 as static (embedded into arcade_sfml.so) ─────────────
message(STATUS "[arcade_sfml] Fetching SFML 3.0.0 (static) ...")
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0 GIT_SHALLOW TRUE)

set(SFML_BUILD_AUDIO            ON  CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK          OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC              OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE       OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
set(SFML_STATIC_LIBRARIES       ON  CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)

set(_SAVED_LIB_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(_SAVED_ARC_DIR "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fetchcontent_libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fetchcontent_libs")
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
FetchContent_MakeAvailable(SFML)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_SAVED_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_SAVED_ARC_DIR}")

# Restore shared libs for games / other targets processed after this
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

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
    message(WARNING "[arcade_sfml] graphics/assets/Inter.ttf not found — text rendering disabled")
endif()

add_library(arcade_sfml SHARED SFMLDisplay.cpp "${_ARCADE_FONT_GEN}")
target_include_directories(arcade_sfml PRIVATE
    ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/graphics ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(arcade_sfml PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)
target_link_libraries(arcade_sfml PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)
target_compile_definitions(arcade_sfml PRIVATE ARCADE_SFML_MAJOR=3)

set_target_properties(arcade_sfml PROPERTIES
    PREFIX "" OUTPUT_NAME "arcade_sfml")
