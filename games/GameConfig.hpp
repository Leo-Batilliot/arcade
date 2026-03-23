#pragma once
#include "Types.hpp"

namespace Arcade {

// ─── Snake ───────────────────────────────────────────────────────────────────
namespace SnakeConfig {

    // Grid dimensions
    constexpr int GRID_WIDTH  = 20;
    constexpr int GRID_HEIGHT = 20;

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

    // Game zone — bounding box as screen ratios [0..1]
    constexpr float ZONE_X1 = 0.20f;
    constexpr float ZONE_Y1 = 0.15f;
    constexpr float ZONE_X2 = 0.80f;
    constexpr float ZONE_Y2 = 0.75f;

    // Grid visual style
    constexpr GridStyle GRID_STYLE = {
        {10,   10,  10,   255}, // backgroundColor: pure black
        10.f,                 // borderPx
        {255, 255, 255, 60},  // highlightColor
    };

} // namespace SnakeConfig

namespace SnakeLayout {
    
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.05f;
    constexpr Color TITLE_COLOR = Colors::Cyan;
    constexpr float TITLE_SCALE = 1.0f;
}

// ─── Minesweeper ─────────────────────────────────────────────────────────────
namespace MinesweeperConfig {

    // Grid dimensions
    constexpr int COLS  = 9;
    constexpr int ROWS  = 9;
    constexpr int MINES = 10;

    // Gameplay
    constexpr float KEY_DELAY        = 0.11f;
    constexpr int   SCORE_PER_REVEAL = 10;

    // Game zone — bounding box as screen ratios [0..1]
    constexpr float ZONE_X1 = 0.20f;
    constexpr float ZONE_Y1 = 0.15f;
    constexpr float ZONE_X2 = 0.80f;
    constexpr float ZONE_Y2 = 0.75f;

    // Grid visual style
    constexpr GridStyle GRID_STYLE = {
        {10,  10,  10,  255}, // backgroundColor: dark gray
        12.f,                 // borderPx
        {255, 255, 255, 80},  // highlightColor (slightly stronger for cursor visibility)
    };

} // namespace MinesweeperConfig

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
