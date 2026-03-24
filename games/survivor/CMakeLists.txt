include(${CMAKE_SOURCE_DIR}/cmake/ArcadeEmbed.cmake)

arcade_embed_asset("assets/survivormusic.ogg")
arcade_embed_asset("assets/explosion.ogg")

add_library(arcade_survivor SHARED
    Survivor.cpp
    ${ARCADE_ASSET_SOURCES}
)

target_include_directories(arcade_survivor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/games
    ${CMAKE_SOURCE_DIR}/graphics
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(arcade_survivor PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)

set_target_properties(arcade_survivor PROPERTIES
    PREFIX ""
    OUTPUT_NAME "arcade_survivor"
)
