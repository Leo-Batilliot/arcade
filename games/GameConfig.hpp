#pragma once
#include "Types.hpp"

namespace Arcade {

// ─── Snake ───────────────────────────────────────────────────────────────────
namespace SnakeConfig {
    constexpr int   GRID_WIDTH            = 20;
    constexpr int   GRID_HEIGHT           = 20;

    // Gameplay
    constexpr int   SCORE_PER_FOOD        = 10;
    constexpr int   SPEEDUP_SCORE_PERIOD  = 50;
    constexpr float INITIAL_STEP_PERIOD   = 0.18f;
    constexpr float MIN_STEP_PERIOD       = 0.06f;
    constexpr float STEP_PERIOD_DECREMENT = 0.015f;
    constexpr int   INITIAL_LENGTH        = 4;

    // Render colors
    constexpr Color BG_DARK    = {10,  10,  10,  255};
    constexpr Color BG_LIGHT   = {22,  22,  22,  255};
    constexpr Color BODY_COLOR = {0,   110, 0,   255};
    constexpr Color FOOD_COLOR = {180, 30,  30,  255};
}

// ─── Minesweeper ─────────────────────────────────────────────────────────────
namespace MinesweeperConfig {
    constexpr int   COLS             = 9;
    constexpr int   ROWS             = 9;
    constexpr int   MINES            = 10;
    constexpr float KEY_DELAY        = 0.11f;
    constexpr int   SCORE_PER_REVEAL = 10;
}

// ── Minesweeper HUD layout (screen ratios, colors, scales) ───────────────────
namespace MinesweeperLayout {

    // Title
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.05f;
    constexpr Color TITLE_COLOR = Colors::Cyan;
    constexpr float TITLE_SCALE = 1.0f;

    // Stats line (mines remaining, time, score)
    constexpr float STATS_X     = 0.5f;
    constexpr float STATS_Y     = 0.10f;
    constexpr Color STATS_COLOR = Colors::White;
    constexpr float STATS_SCALE = 1.0f;

    // Controls hint
    constexpr float CONTROLS_X     = 0.5f;
    constexpr float CONTROLS_Y     = 0.82f;
    constexpr Color CONTROLS_COLOR = Colors::Gray;
    constexpr float CONTROLS_SCALE = 1.0f;

} // namespace MinesweeperLayout

} // namespace Arcade
