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

    // Input: same direction key can't fire twice within this delay (prevents OS key-repeat accidents)
    constexpr float KEY_REPEAT_DELAY = 999.f; // effectively once per game — no use case for same arrow twice

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
    constexpr float CONTROLS_Y     = 0.83f;
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
    constexpr float KEY_REPEAT_DELAY = 0.30f; // same key can't fire again within this delay (prevents accidental holds)
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
    constexpr float CONTROLS_Y     = 0.83f;
    constexpr Color CONTROLS_COLOR = Colors::Gray;
    constexpr float CONTROLS_SCALE = 1.0f;

} // namespace MinesweeperLayout

// ─── Survivor ────────────────────────────────────────────────────────────────
namespace SurvivorConfig {

    // ── Grid ──────────────────────────────────────────────────────────────────
    constexpr int GRID_W = 100;
    constexpr int GRID_H = 100;

    // ── Player ────────────────────────────────────────────────────────────────
    constexpr int   INITIAL_HEALTH        = 5;
    constexpr int   PLAYER_UPPER_LIMIT    = 15;   ///< topmost Y the player can reach
    constexpr int   INITIAL_PLAYER_Y      = 80;
    constexpr int   PLAYER_RIGHT_MARGIN   = 6;    ///< x < GRID_W - PLAYER_RIGHT_MARGIN
    constexpr int   PLAYER_BOTTOM_MARGIN  = 8;    ///< y < GRID_H - PLAYER_BOTTOM_MARGIN
    constexpr float PLAYER_SPEED         = 50.f;  ///< cells per second (delta-time movement)
    constexpr int   PLAYER_SPRITE_HALF_W = 3;     ///< half of player sprite width (6/2) for centering

    // ── Enemy ─────────────────────────────────────────────────────────────────
    constexpr int   INITIAL_ENEMY_Y         = 5;
    constexpr int   ENEMY_PROJ_OFFSET_X     = 8;
    constexpr int   ENEMY_PROJ_OFFSET_Y     = 8;
    constexpr int   ENEMY_RANDOM_MOVE_COUNT = 10;
    constexpr int   ENEMY_RANDOMNESS_DISTANCE = 20;
    constexpr int   ENEMY_SPRITE_HALF_W    = 9;   ///< half of enemy sprite width (19/2) for centering

    // ── Timing (seconds) ─────────────────────────────────────────────────────
    constexpr float MOVE_PERIOD           = 0.02f;
    constexpr float PROJ_MOVE_PERIOD      = 0.05f;
    constexpr float STAR_MOVE_PERIOD      = 0.05f;
    constexpr float PROJ_SPAWN_PERIOD     = 0.8f;
    constexpr float ENEMY_MOVE_TIMER      = 0.05f;

    // ── Mouse tracking ────────────────────────────────────────────────────
    constexpr float MOUSE_ACTIVE_DURATION = 0.4f;  ///< seconds after last move that mouse tracking is active
    constexpr float MOUSE_DEAD_ZONE       = 1.0f;  ///< grid-cell radius — ship stops chasing inside dead zone

    // ── Difficulty progression ────────────────────────────────────────────
    constexpr float MAX_DIFFICULTY           = 20.f; ///< ceiling capping all scaling formulas
    constexpr float DIFFICULTY_LEVEL_DURATION = 10.f; ///< seconds per difficulty level (+1 every 15s)
    constexpr float PROJ_SPAWN_SPEED_MULT = 0.4f;  ///< how much difficulty reduces spawn period
    constexpr float ENEMY_SPEED_MULT      = 1.0f;  ///< how much difficulty increases enemy move speed
    constexpr float MIN_SPAWN_PERIOD      = 0.03f; ///< floor for spawn period
    constexpr float MIN_ENEMY_MOVE_PERIOD = 0.03f; ///< floor for enemy move period
    constexpr float PROJ_SPEED_MULT       = 0.5f;  ///< how much difficulty increases proj vy
    constexpr int   MAX_PROJ_VY           = 15;    ///< ceiling for projectile vy

    // ── Projectiles ───────────────────────────────────────────────────────────
    constexpr int   PROJ_MIN_SIZE         = 2;
    constexpr int   PROJ_SIZE_RANGE       = 4;    ///< size = MIN + rand % RANGE
    constexpr int   PROJ_VX_RANGE         = 3;    ///< vx = rand%RANGE - RANGE/2

    // ── Stars ─────────────────────────────────────────────────────────────────
    constexpr int   INITIAL_STAR_COUNT    = 40;   ///< pre-seeded across whole grid at init
    constexpr int   STARS_PER_LINE_AVERAGE = 3;    ///< 1-in-N chance to spawn per star tick

    // ── Effects ───────────────────────────────────────────────────────────────
    constexpr float EXPLOSION_DURATION    = 0.35f;
    constexpr int   TRAIL_LENGTH          = 8;
    constexpr float TRAIL_UPDATE_PERIOD   = 0.04f; ///< how often a new trail particle is added
    constexpr int   EXHAUST_OFFSET_X      = 2;   ///< nozzle col relative to player origin
    constexpr int   EXHAUST_OFFSET_Y      = 7;   ///< nozzle row relative to player origin

    // ── Colors ────────────────────────────────────────────────────────────────
    constexpr Color PLAYER_COLOR          = {0,   255, 0,   255};
    constexpr Color ENEMY_COLOR           = {255, 0,   0,   255};
    constexpr Color STAR_COLOR            = {200, 200, 0,   255};
    constexpr Color PROJ_COLOR            = {255, 100, 0,   255}; // square
    constexpr Color PROJ_CIRCLE_COLOR     = {100, 200, 255, 255}; // circle
    constexpr Color PROJ_TRIANGLE_COLOR   = {255, 50,  150, 255}; // triangle
    constexpr Color PROJ_LINE_COLOR       = {255, 255, 100, 255}; // line
    constexpr Color EXPLOSION_COLOR       = {255, 200, 50,  255};
    constexpr Color EXPLOSION_CORE_COLOR  = {255, 255, 200, 255};
    constexpr Color TRAIL_COLOR           = {220, 40,  40,  255};
    constexpr Color BACKGROUND_COLOR      = {4, 10, 20, 255};

    // ── Sprites ───────────────────────────────────────────────────────────────
    const std::vector<std::string> PLAYER_ASCII = {
        "  /\\  ",
        "  ||  ",
        "  ||  ",
        " /||\\ ",
        "/:||:\\",
        "|:||:|",
        "|/||\\|"
    };

    const std::vector<std::string> ENEMY_ASCII = {
        "         .         ",
        "         |         ",
        "      .-\"^\"-.      ",
        "     /_....._\\     ",
        " .-\"`         `\"-. ",
        "(  ooo  ooo  ooo  )",
        " '-.,_________,.-' ",
        "     /       \\     "
    };

    const std::vector<std::string> EXPLOSION_ASCII = {
        "\\|/",
        "-o-",
        "/|\\"
    };

    // ── Game zone ─────────────────────────────────────────────────────────────
    constexpr float ZONE_X1 = 0.2f;
    constexpr float ZONE_Y1 = 0.1f;
    constexpr float ZONE_X2 = 0.8f;
    constexpr float ZONE_Y2 = 0.85f;

    // ── Grid visual style ─────────────────────────────────────────────────────
    constexpr GridStyle GRID_STYLE = {
        {10, 10, 10, 255},
        10.f,
        {255, 255, 255, 60},
    };

} // namespace SurvivorConfig

namespace SurvivorLayout {

    // Title
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.02f;
    constexpr Color TITLE_COLOR = Colors::Cyan;
    constexpr float TITLE_SCALE = 2.0f;

    // Stats (score / time / difficulty)
    constexpr float STATS_X     = 0.5f;
    constexpr float STATS_Y     = 0.06f;
    constexpr Color STATS_COLOR = Colors::White;
    constexpr float STATS_SCALE = 1.0f;

    // HP bar: label left of bar, bar spans BAR_X1..BAR_X2
    constexpr float HP_LABEL_X    = 0.23f;
    constexpr float HP_BAR_X1     = 0.25f;
    constexpr float HP_BAR_X2     = 0.75f;
    constexpr float HP_BAR_Y      = 0.08f;
    constexpr Color HP_FG_COLOR   = Colors::White;
    constexpr Color HP_BG_FULL    = {180, 0,   0,   255}; // filled HP (deep red)
    constexpr Color HP_BG_EMPTY   = {50,  0,   0,   255}; // empty HP (very dark red)
    constexpr float HP_SCALE      = 1.0f;

    // Controls hint
    constexpr float CONTROLS_X     = 0.5f;
    constexpr float CONTROLS_Y     = 0.90f;
    constexpr Color CONTROLS_COLOR = Colors::Gray;
    constexpr float CONTROLS_SCALE = 1.0f;

} // namespace SurvivorLayout

} // namespace Arcade
