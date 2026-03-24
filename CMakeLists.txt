cmake_minimum_required(VERSION 3.16)
project(Arcade VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/archive)

# Suppress FetchContent/ExternalProject status noise from downloaded libs
set(FETCHCONTENT_QUIET           ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
include(FetchContent)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(graphics/ncurses)
add_subdirectory(graphics/sfml)
add_subdirectory(graphics/sdl2)
add_subdirectory(games/snake)
add_subdirectory(games/minesweeper)
add_subdirectory(games/survivor)

add_custom_target(core       DEPENDS arcade                                  COMMENT "arcade executable")
add_custom_target(graphicals DEPENDS arcade_ncurses arcade_sfml arcade_sdl2  COMMENT "display libs")
add_custom_target(games      DEPENDS arcade_snake arcade_minesweeper arcade_survivor    COMMENT "game libs")

message(STATUS "")
message(STATUS "=== Arcade ===  build type: ${CMAKE_BUILD_TYPE}  |  C++${CMAKE_CXX_STANDARD}")
message(STATUS "  arcade       -> ${CMAKE_SOURCE_DIR}/arcade")
message(STATUS "  libs         -> ${CMAKE_SOURCE_DIR}/lib/")
message(STATUS "  targets      -> core  graphicals  games")
message(STATUS "")
