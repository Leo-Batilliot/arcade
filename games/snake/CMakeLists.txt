include(${CMAKE_SOURCE_DIR}/cmake/ArcadeEmbed.cmake)

arcade_embed_asset("assets/snakemusic.ogg")
arcade_embed_asset("assets/move.ogg")
arcade_embed_asset("assets/eat.ogg")
arcade_embed_asset("assets/Speed_up.ogg")
arcade_embed_asset("assets/snake_head.png")
arcade_embed_asset("assets/snake_straight_body.png")
arcade_embed_asset("assets/snake_curved_body.png")
arcade_embed_asset("assets/snake_tail.png")
arcade_embed_asset("assets/apple.png")
arcade_embed_asset("assets/orange.png")

add_library(arcade_snake SHARED
    Snake.cpp
    ${ARCADE_ASSET_SOURCES}
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
