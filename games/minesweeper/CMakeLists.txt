include(${CMAKE_SOURCE_DIR}/cmake/ArcadeEmbed.cmake)

arcade_embed_asset("assets/minesweeper.ogg")
arcade_embed_asset("assets/Flag.ogg")
arcade_embed_asset("assets/Safe_cell.ogg")
arcade_embed_asset("assets/Bomb.ogg")
arcade_embed_asset("assets/Next_level.ogg")
arcade_embed_asset("assets/flag.jpg")
arcade_embed_asset("assets/bomb.png")
arcade_embed_asset("assets/unknown_cell.png")

add_library(arcade_minesweeper SHARED
    Minesweeper.cpp
    ${ARCADE_ASSET_SOURCES}
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
