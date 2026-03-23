#pragma once
#include "../graphics/DisplayConfig.hpp"
#include "../include/Types.hpp"

namespace Arcade {
namespace CoreConfig {

    // ── Library scanning ──────────────────────────────────────────────────────
    constexpr const char* LIB_DIR    = "./lib";
    constexpr const char* LIB_EXT   = ".so";
    constexpr const char* LIB_PREFIX = "arcade_";

    // ── Score persistence ─────────────────────────────────────────────────────
    constexpr const char* SCORES_FILE        = "scores.txt";
    constexpr int         MAX_SCORES_PER_GAME = 10;

    // ── Player name ───────────────────────────────────────────────────────────
    constexpr int PLAYER_NAME_MAX_LEN = 20;

    // ── Volume ────────────────────────────────────────────────────────────────
    constexpr int VOLUME_STEP = 10;

    // ── Frame timing ──────────────────────────────────────────────────────────
    constexpr int   MENU_FRAME_MS     = 16;
    constexpr int   GAMEOVER_FRAME_MS = 16;
    constexpr int   LOADING_DELAY_MS  = 500;
    constexpr float MAX_DELTA_TIME    = 0.1f;

    // ── HUD separator line ────────────────────────────────────────────────────
    constexpr char  HUD_LINE_CHAR        = '-';
    constexpr float HUD_LINE_X_START     = 0.0f;
    constexpr float HUD_LINE_X_END       = 1.0f;
    constexpr float HUD_LINE_Y           = 0.85f;
    constexpr Color HUD_LINE_COLOR       = Colors::DarkGray;
    constexpr Color HUD_LINE_BG_COLOR    = Colors::Black;

    // ── HUD score / game info (center) ────────────────────────────────────────
    constexpr float HUD_SCORE_TEXT_X     = 0.5f;
    constexpr float HUD_SCORE_TEXT_Y     = 0.90f;
    constexpr Color HUD_SCORE_TEXT_COLOR = Colors::White;
    constexpr float HUD_SCORE_TEXT_SCALE = 1.0f;

    // ── HUD shortcuts (right side hints) ──────────────────────────────────────
    constexpr float HUD_KEYS_TEXT_X        = 0.75f;
    constexpr float HUD_KEYS_TEXT_Y        = 0.90f;
    constexpr Color HUD_KEYS_TEXT_COLOR    = Colors::White;
    constexpr Color HUD_KEYS_TEXT_BG_COLOR = Colors::Black;
    constexpr float HUD_KEYS_TEXT_SCALE    = 1.0f;

    // ── HUD TAB hint (left) ───────────────────────────────────────────────────
    constexpr float HUD_TAB_TEXT_X        = 0.25f;
    constexpr float HUD_TAB_TEXT_Y        = 0.90f;
    constexpr Color HUD_TAB_TEXT_COLOR    = Colors::White;
    constexpr Color HUD_TAB_TEXT_BG_COLOR = Colors::Black;
    constexpr float HUD_TAB_TEXT_SCALE    = 1.0f;

    // ── Persistent volume indicator (top-right corner) ────────────────────────
    constexpr float VOLUME_TEXT_X     = 0.93f;
    constexpr float VOLUME_TEXT_Y     = 0.03f;
    constexpr Color VOLUME_TEXT_COLOR = Colors::Gray;
    constexpr float VOLUME_TEXT_SCALE = 0.85f;

} // namespace CoreConfig

// ── Menu layout (screen ratios, colors, scales) ───────────────────────────────
namespace MenuLayout {

    // Boundary between columns for mouse hit-testing
    constexpr float LEFT_ZONE_BOUNDARY  = 0.42f;
    constexpr float RIGHT_ZONE_BOUNDARY = 0.58f;

    // Title
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.06f;
    constexpr Color TITLE_COLOR = Colors::Yellow;
    constexpr float TITLE_SCALE = 1.8f;

    // Player name input (center, above scores)
    constexpr float PLAYER_NAME_X     = 0.5f;
    constexpr float PLAYER_NAME_Y     = 0.12f;
    constexpr Color PLAYER_NAME_COLOR = Colors::Cyan;
    constexpr float PLAYER_NAME_SCALE = 1.0f;

    // Left column — Games
    constexpr float GAMES_HDR_X          = 0.22f;
    constexpr float GAMES_HDR_Y          = 0.16f;
    constexpr Color GAMES_HDR_COLOR      = Colors::White;
    constexpr float GAMES_HDR_SCALE      = 1.2f;
    constexpr float GAMES_ITEM_X         = 0.22f;
    constexpr float GAMES_START_Y        = 0.23f;
    constexpr float GAMES_STEP_Y         = 0.062f;
    constexpr Color GAMES_SELECTED_COLOR = Colors::Green;
    constexpr Color GAMES_COLOR          = Colors::Gray;

    // Center column — Scores
    constexpr float SCORES_HDR_X      = 0.5f;
    constexpr float SCORES_HDR_Y      = 0.16f;
    constexpr Color SCORES_HDR_COLOR  = Colors::White;
    constexpr float SCORES_HDR_SCALE  = 1.2f;
    constexpr float SCORES_ITEM_X     = 0.5f;
    constexpr float SCORES_START_Y    = 0.23f;
    constexpr float SCORES_STEP_Y     = 0.062f;
    constexpr Color SCORES_COLOR      = Colors::Yellow;
    constexpr Color SCORES_NAME_COLOR = Colors::Gray;

    // Right column — Graphics
    constexpr float GFX_HDR_X          = 0.78f;
    constexpr float GFX_HDR_Y          = 0.16f;
    constexpr Color GFX_HDR_COLOR      = Colors::White;
    constexpr float GFX_HDR_SCALE      = 1.2f;
    constexpr float GFX_ITEM_X         = 0.78f;
    constexpr float GFX_START_Y        = 0.23f;
    constexpr float GFX_STEP_Y         = 0.062f;
    constexpr Color GFX_SELECTED_COLOR = Colors::LightBlue;
    constexpr Color GFX_ACTIVE_COLOR   = Colors::Cyan;
    constexpr Color GFX_COLOR          = Colors::Gray;

    // Loading overlay
    constexpr float LOADING_X     = 0.5f;
    constexpr float LOADING_Y     = 0.5f;
    constexpr Color LOADING_COLOR = Colors::Yellow;
    constexpr float LOADING_SCALE = 1.0f;

    // Footer hints
    constexpr float FOOTER1_X     = 0.5f;
    constexpr float FOOTER1_Y     = 0.90f;
    constexpr Color FOOTER1_COLOR = Colors::White;
    constexpr float FOOTER2_X     = 0.5f;
    constexpr float FOOTER2_Y     = 0.95f;
    constexpr Color FOOTER2_COLOR = Colors::Gray;

} // namespace MenuLayout

// ── Game-over screen layout ───────────────────────────────────────────────────
namespace GameOverLayout {

    // Semi-transparent overlay behind the box
    constexpr Color OVERLAY_COLOR  = {0, 0, 0, 210};

    // Bounding box lines
    constexpr char  BOX_LINE_CHAR  = '=';
    constexpr float BOX_LINE_X1    = 0.25f;
    constexpr float BOX_LINE_X2    = 0.75f;
    constexpr float BOX_TOP_LINE_Y = 0.38f;
    constexpr float BOX_BOT_LINE_Y = 0.64f;
    constexpr Color BOX_WON_COLOR  = Colors::Green;
    constexpr Color BOX_LOST_COLOR = Colors::Red;

    // Title ("GAME OVER" / "CONGRATULATIONS!")
    constexpr float TITLE_X     = 0.5f;
    constexpr float TITLE_Y     = 0.44f;
    constexpr float TITLE_SCALE = 1.6f;

    // Game name
    constexpr float GAME_NAME_X     = 0.5f;
    constexpr float GAME_NAME_Y     = 0.50f;
    constexpr Color GAME_NAME_COLOR = Colors::Gray;
    constexpr float GAME_NAME_SCALE = 1.0f;

    // Score
    constexpr float SCORE_X     = 0.5f;
    constexpr float SCORE_Y     = 0.55f;
    constexpr Color SCORE_COLOR = Colors::Yellow;
    constexpr float SCORE_SCALE = 1.2f;

    // Continue prompt
    constexpr float CONTINUE_X     = 0.5f;
    constexpr float CONTINUE_Y     = 0.61f;
    constexpr Color CONTINUE_COLOR = Colors::White;
    constexpr float CONTINUE_SCALE = 1.0f;

} // namespace GameOverLayout

} // namespace Arcade
