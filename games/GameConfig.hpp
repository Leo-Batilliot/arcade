#pragma once
#include "Types.hpp"

namespace Arcade {

// ─── Snake ───────────────────────────────────────────────────────────────────
namespace SnakeConfig {

    // Grid dimensions Keep approximatively a 16/9 ratio for coherent display on screen
    constexpr int GRID_WIDTH  = 25;
    constexpr int GRID_HEIGHT = 14;

    // Gameplay
    constexpr int   SCORE_PER_FOOD          = 10;
    constexpr int   ORANGE_SCORE_MULTIPLIER = 2;
    constexpr float STEP_PERIOD             = 0.12f;  // fixed speed (no progression)
    constexpr float ORANGE_SPEED_MULTIPLIER = 2.0f;   // orange halves the step period
    constexpr float ORANGE_BOOST_DURATION   = 3.0f;   // seconds of speed boost after eating orange
    constexpr int   MAX_APPLES              = 5;
    constexpr int   MAX_ORANGES             = 1;
    constexpr int   INITIAL_LENGTH          = 4;

    // Render colors
    constexpr Color BG_DARK     = {15,  15,  15,  255};
    constexpr Color BG_LIGHT    = {22,  22,  22,  255};
    constexpr Color BODY_COLOR  = {0,   110, 0,   255};
    constexpr Color FOOD_COLOR  = {180, 30,  30,  255};
    constexpr Color ORANGE_COLOR = {210, 100, 0,   255};

    // Game zone — bounding box as screen ratios [0..1]
    constexpr float ZONE_X1 = 0.2f;
    constexpr float ZONE_Y1 = 0.2f;
    constexpr float ZONE_X2 = 0.8f;
    constexpr float ZONE_Y2 = 0.8f;

    // Grid visual style
    constexpr GridStyle GRID_STYLE = {
        {10,   10,  10,   255}, // backgroundColor: pure black
        10.f,                 // borderPx
        {255, 255, 255, 60},  // highlightColor
    };

} // namespace SnakeConfig

namespace SnakeLayout {
    
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.02f;
    constexpr Color TITLE_COLOR = Colors::Cyan;
    constexpr float TITLE_SCALE = 2.0f;

    // Stats line (snake length, time, score)
    constexpr float STATS_X     = 0.5f;
    constexpr float STATS_Y     = 0.15f;
    constexpr Color STATS_COLOR = Colors::White;
    constexpr float STATS_SCALE = 2.0f;

    // Controls hint
    constexpr float CONTROLS_X     = 0.5f;
    constexpr float CONTROLS_Y     = 0.81f;
    constexpr Color CONTROLS_COLOR = Colors::Gray;
    constexpr float CONTROLS_SCALE = 1.0f;
}

// ─── Minesweeper ─────────────────────────────────────────────────────────────
namespace MinesweeperConfig {

    // Level progression
    constexpr int   INITIAL_SIZE = 7;     // starting grid side length (odd)
    constexpr float MINE_RATIO   = 0.16f; // mines as fraction of total cells
    constexpr float LEVEL_GROWTH = 1.33f; // grid side growth factor per level

    // Gameplay
    constexpr float KEY_DELAY        = 0.11f;
    constexpr int   SCORE_PER_REVEAL = 10;

    // Game zone — bounding box as screen ratios [0..1]
    constexpr float ZONE_X1 = 0.34f;
    constexpr float ZONE_Y1 = 0.20f;
    constexpr float ZONE_X2 = 0.66f;
    constexpr float ZONE_Y2 = 0.78f;

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
    constexpr float TITLE_Y     = 0.02f;
    constexpr Color TITLE_COLOR = Colors::Cyan;
    constexpr float TITLE_SCALE = 2.0f;

    // Stats line (mines remaining, time, score)
    constexpr float STATS_X     = 0.5f;
    constexpr float STATS_Y     = 0.15f;
    constexpr Color STATS_COLOR = Colors::White;
    constexpr float STATS_SCALE = 2.0f;

    // Controls hint
    constexpr float CONTROLS_X     = 0.5f;
    constexpr float CONTROLS_Y     = 0.81f;
    constexpr Color CONTROLS_COLOR = Colors::Gray;
    constexpr float CONTROLS_SCALE = 1.0f;

} // namespace MinesweeperLayout

} // namespace Arcade
