#pragma once
#include <cstdint>

namespace Arcade {
namespace DisplayConfig {

    // ── Window ────────────────────────────────────────────────────────────────
    constexpr int   WIN_W = 1920;
    constexpr int   WIN_H = 1080;

    // ── Game zone: center position (fraction of screen) ───────────────────────
    constexpr float GAME_ZONE_CENTER_X     = 0.50f;
    constexpr float GAME_ZONE_CENTER_Y     = 0.45f;

    // ── Game zone: size (fraction of screen) ──────────────────────────────────
    constexpr float GAME_ZONE_WIDTH_RATIO  = 1.00f;
    constexpr float GAME_ZONE_HEIGHT_RATIO = 0.60f;

    // ── Minimum cell size (pixel-based backends) ──────────────────────────────
    constexpr float MIN_CELL_PX = 12.f;

    // ── Base font size for pixel backends (used as multiplier base) ───────────
    constexpr int BASE_FONT_SIZE = 20;

    // ── Grid background: border padding and fill color ────────────────────────
    constexpr float GRID_BORDER_PX      = 12.f;
    constexpr int   GRID_BG_R           = 20;
    constexpr int   GRID_BG_G           = 20;
    constexpr int   GRID_BG_B           = 20;

    // ── Cell highlight overlay (semi-transparent white) ───────────────────────
    constexpr int HIGHLIGHT_R = 255;
    constexpr int HIGHLIGHT_G = 255;
    constexpr int HIGHLIGHT_B = 255;
    constexpr int HIGHLIGHT_A = 60;

    // ── Derived game zone bounds (ratios — no pixels) ─────────────────────────
    constexpr float GAME_ZONE_X1 = GAME_ZONE_CENTER_X - GAME_ZONE_WIDTH_RATIO  / 2.f; // 0.00
    constexpr float GAME_ZONE_Y1 = GAME_ZONE_CENTER_Y - GAME_ZONE_HEIGHT_RATIO / 2.f; // 0.15
    constexpr float GAME_ZONE_X2 = GAME_ZONE_CENTER_X + GAME_ZONE_WIDTH_RATIO  / 2.f; // 1.00
    constexpr float GAME_ZONE_Y2 = GAME_ZONE_CENTER_Y + GAME_ZONE_HEIGHT_RATIO / 2.f; // 0.75

} // namespace DisplayConfig

// ── NCurses-specific constants ────────────────────────────────────────────────
namespace NCursesConfig {

    constexpr int ESCAPE_DELAY_MS         = 25;
    constexpr int DEFAULT_TERMINAL_WIDTH  = 80;
    constexpr int DEFAULT_TERMINAL_HEIGHT = 30;
    constexpr int MAX_CHARS_PER_CELL      = 4;
    constexpr int MIN_CHARS_PER_CELL      = 2;
    constexpr int RESERVED_COLOR_PAIRS    = 8;

} // namespace NCursesConfig

// ── SFML-specific constants ───────────────────────────────────────────────────
namespace SFMLConfig {

    constexpr int FRAMERATE_LIMIT = 60;

} // namespace SFMLConfig

// ── SDL2-specific constants ───────────────────────────────────────────────────
namespace SDL2Config {

    constexpr int AUDIO_FREQUENCY  = 44100;
    constexpr int AUDIO_CHANNELS   = 2;
    constexpr int AUDIO_CHUNK_SIZE = 2048;

} // namespace SDL2Config

} // namespace Arcade
