#pragma once
#include <cstdint>
#include <string>

namespace Arcade {

// ─────────────────────────────────────────────────────────────────────────────
// Keys
// ─────────────────────────────────────────────────────────────────────────────
enum class Key {
    NONE,
    // Navigation
    UP, DOWN, LEFT, RIGHT,
    // Common control keys
    ENTER, ESCAPE, SPACE, BACKSPACE, TAB,
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Symbol keys
    PLUS, MINUS,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Number pad (0-9)
    NUM_0, NUM_1, NUM_2, NUM_3, NUM_4,
    NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,
    // Mouse buttons
    MOUSE_LEFT, MOUSE_MIDDLE, MOUSE_RIGHT
};

// ─────────────────────────────────────────────────────────────────────────────
// Mouse Events
// ─────────────────────────────────────────────────────────────────────────────
/// Mouse click/hover event.
/// relativeX/relativeY are screen-ratio positions [0..1] (always set).
/// gridX/gridY are game-grid cell coordinates from the last drawGrid() call
/// (-1 if outside the drawn grid area).
struct MouseEvent {
    float relativeX = 0.f;    ///< Screen X as fraction [0..1]
    float relativeY = 0.f;    ///< Screen Y as fraction [0..1]
    int   gridX     = -1;     ///< Cell column in the last drawGrid (−1 = outside grid)
    int   gridY     = -1;     ///< Cell row in the last drawGrid (−1 = outside grid)
    Key   button    = Key::NONE; ///< Mouse button pressed
};

// ─────────────────────────────────────────────────────────────────────────────
// Color (RGBA)
// ─────────────────────────────────────────────────────────────────────────────
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

namespace Colors {
    constexpr Color Black     = {0,   0,   0,   255};
    constexpr Color White     = {255, 255, 255, 255};
    constexpr Color Red       = {220, 50,  50,  255};
    constexpr Color Green     = {50,  200, 50,  255};
    constexpr Color Blue      = {50,  80,  220, 255};
    constexpr Color Yellow    = {255, 220, 0,   255};
    constexpr Color Cyan      = {0,   200, 220, 255};
    constexpr Color Magenta   = {200, 0,   200, 255};
    constexpr Color Orange    = {255, 140, 0,   255};
    constexpr Color DarkGray  = {40,  40,  40,  255};
    constexpr Color Gray      = {120, 120, 120, 255};
    constexpr Color LightBlue = {100, 149, 237, 255};
    constexpr Color DarkRed   = {80,  0,   0,   255};
    constexpr Color DarkGreen = {0,   60,  0,   255};
}

// ─────────────────────────────────────────────────────────────────────────────
// Rendering primitives
// ─────────────────────────────────────────────────────────────────────────────

/// One cell in a game grid — character, foreground and background color.
struct Cell {
    char  c       = ' ';             ///< Character to display (space = empty/bg-only)
    Color fg      = Colors::White;   ///< Foreground (text) color
    Color bg      = Colors::Black;   ///< Background color for char/text rendering
    Color bgImage = Colors::Black;   ///< Background color drawn behind the image
    const unsigned char* imageData = nullptr; ///< Optional PNG image bytes. nullptr = use char.
    unsigned int         imageSize = 0;       ///< Byte length of imageData.
    float imageAngle = 0.f;          ///< Clockwise rotation in degrees applied to the image.
    bool  imageFlipX = false;        ///< Mirror the image horizontally before rotation.
    bool  highlight  = false;        ///< Draw a semi-transparent overlay on top (hover/selection hint).
};

} // namespace Arcade
