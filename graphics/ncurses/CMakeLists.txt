include(ExternalProject)

find_package(Curses QUIET)

if(CURSES_FOUND)
    add_library(arcade_ncurses SHARED NCursesDisplay.cpp)
    target_include_directories(arcade_ncurses PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/graphics
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CURSES_INCLUDE_DIRS})
    target_link_libraries(arcade_ncurses PRIVATE ${CURSES_LIBRARIES})
else()
    message(STATUS "[arcade_ncurses] Downloading ncurses 6.4 ...")
    set(NCURSES_INSTALL_DIR "${CMAKE_BINARY_DIR}/ncurses_install")

    ExternalProject_Add(ncurses_external
        URL         https://ftp.gnu.org/gnu/ncurses/ncurses-6.4.tar.gz
        URL_HASH    SHA256=6931283d9ac87c5073f30b6290c4c75f21632bb4fc3603ac8100812bed248159
        CONFIGURE_COMMAND
            <SOURCE_DIR>/configure
                --prefix=${NCURSES_INSTALL_DIR}
                --with-shared
                --without-ada
                --without-manpages
                --without-tests
                --enable-widec
                --with-termlib
        BUILD_COMMAND   make -j4
        INSTALL_COMMAND make install
        BUILD_BYPRODUCTS "${NCURSES_INSTALL_DIR}/lib/libncursesw.so"
        LOG_CONFIGURE 1
        LOG_BUILD     1
        LOG_INSTALL   1
    )

    add_library(ncurses_imported SHARED IMPORTED)
    set_target_properties(ncurses_imported PROPERTIES
        IMPORTED_LOCATION "${NCURSES_INSTALL_DIR}/lib/libncursesw.so")
    add_dependencies(ncurses_imported ncurses_external)

    add_library(arcade_ncurses SHARED NCursesDisplay.cpp)
    target_include_directories(arcade_ncurses PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/graphics
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${NCURSES_INSTALL_DIR}/include"
        "${NCURSES_INSTALL_DIR}/include/ncursesw")
    target_link_libraries(arcade_ncurses PRIVATE ncurses_imported)
    add_dependencies(arcade_ncurses ncurses_external)
endif()

target_compile_options(arcade_ncurses PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)

set_target_properties(arcade_ncurses PROPERTIES
    PREFIX      ""
    OUTPUT_NAME "arcade_ncurses")
