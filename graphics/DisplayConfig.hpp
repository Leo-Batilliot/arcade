#pragma once
#include <cstdint>

namespace Arcade {
namespace DisplayConfig {

    // ── Window ────────────────────────────────────────────────────────────────
    constexpr int WIN_W = 1920;
    constexpr int WIN_H = 1080;

    // ── Minimum cell size (pixel-based backends) ──────────────────────────────
    // The display enforces this floor so cells never become unreadably tiny
    // regardless of grid size.  Games cannot go below this physical limit.
    constexpr float MIN_CELL_PX = 12.f;

    // ── Base font size for pixel backends (used as multiplier base) ───────────
    constexpr int BASE_FONT_SIZE = 20;

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
