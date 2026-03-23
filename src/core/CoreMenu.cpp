#include "Core.hpp"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <thread>

namespace {

std::string formatDuration(int secs)
{
    if (secs < 60)
        return std::to_string(secs) + "secs";
    const int mins = secs / 60;
    const int rem  = secs % 60;
    if (rem == 0)
        return std::to_string(mins) + "mins";
    return std::to_string(mins) + "mins " + std::to_string(rem) + "secs";
}

} // namespace

namespace Arcade {

void Core::runMenu()
{
    auto lastTime = std::chrono::steady_clock::now();

    while (_state == State::MENU && _display && _display->isOpen()) {
        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        // ── Blink cursor ──────────────────────────────────────────────────────
        if (_nameInputActive) {
            _cursorBlinkTimer += deltaTime;
            if (_cursorBlinkTimer >= MenuLayout::PLAYER_NAME_BLINK_PERIOD) {
                _cursorBlinkTimer  -= MenuLayout::PLAYER_NAME_BLINK_PERIOD;
                _cursorBlinkVisible = !_cursorBlinkVisible;
            }
        } else {
            _cursorBlinkTimer   = 0.f;
            _cursorBlinkVisible = true;
        }

        const auto keys        = _display->getKeys();
        const auto mouseEvents = _display->getMouseEvents();
        const std::string typed = _display->getTextInput();

        handleVolumeKeys(keys);

        // Text input only when name field is active
        if (_nameInputActive) {
            for (char ch : typed) {
                if (std::isprint((unsigned char)ch) && ch != '+' && ch != '-'
                    && _playerName.size() < CoreConfig::PLAYER_NAME_MAX_LEN)
                    _playerName += ch;
            }
        }

        for (const auto key : keys) {
            if (key == Key::ESCAPE) {
                _running = false;
                return;
            }
            if (key == Key::TAB) {
                _currentDisplay = (_currentDisplay + 1) % _displayLibs.size();
                renderLoading(_displayLibs[_currentDisplay].name);
                loadDisplay(_currentDisplay);
                return;
            }
            if (key == Key::BACKSPACE && _nameInputActive && !_playerName.empty()) {
                _playerName.pop_back();
                continue;
            }
            if (key == Key::UP && !_nameInputActive)
                _menuGameCursor = std::max(0, _menuGameCursor - 1);
            if (key == Key::DOWN && !_nameInputActive)
                _menuGameCursor = std::min((int)_gameLibs.size() - 1, _menuGameCursor + 1);
            if (key == Key::ENTER) {
                if (_nameInputActive) {
                    _nameInputActive = false;
                } else if (!_gameLibs.empty()) {
                    _currentGame = _menuGameCursor;
                    loadGame(_currentGame);
                    _state = State::PLAYING;
                    return;
                }
            }
        }

        for (const auto& mouseEvent : mouseEvents) {
            if (mouseEvent.button == Key::MOUSE_LEFT)
                handleMenuMouseClick(mouseEvent.relativeX, mouseEvent.relativeY);
        }

        handleMenuMouseHover();
        renderMenu();
        std::this_thread::sleep_for(std::chrono::milliseconds(CoreConfig::MENU_FRAME_MS));
    }
}

void Core::handleMenuMouseHover()
{
    if (!_display->hasMouseMoved())
        return;

    auto [relativeX, relativeY] = _display->getMousePosition();

    if (relativeX < 0.f)
        return;

    // Player name field hover
    _nameInputHovered =
        std::abs(relativeY - MenuLayout::PLAYER_NAME_Y) < MenuLayout::PLAYER_NAME_ZONE_Y_HALF;

    // Left column: games
    if (relativeX < MenuLayout::LEFT_ZONE_BOUNDARY) {
        for (int i = 0; i < (int)_gameLibs.size(); ++i) {
            float itemY = MenuLayout::GAMES_START_Y + i * MenuLayout::GAMES_STEP_Y;
            if (std::abs(relativeY - itemY) < MenuLayout::GAMES_STEP_Y * 0.5f) {
                _menuGameCursor = i;
                _menuGraphicsCursor = -1;
                return;
            }
        }
    }

    // Right column: graphics
    if (relativeX > MenuLayout::RIGHT_ZONE_BOUNDARY) {
        for (int i = 0; i < (int)_displayLibs.size(); ++i) {
            float itemY = MenuLayout::GFX_START_Y + i * MenuLayout::GFX_STEP_Y;
            if (std::abs(relativeY - itemY) < MenuLayout::GFX_STEP_Y * 0.5f) {
                _menuGraphicsCursor = i;
                return;
            }
        }
    }

    _menuGraphicsCursor = -1;
}

void Core::handleMenuMouseClick(float relativeX, float relativeY)
{
    // Player name field
    if (std::abs(relativeY - MenuLayout::PLAYER_NAME_Y) < MenuLayout::PLAYER_NAME_ZONE_Y_HALF) {
        _nameInputActive    = true;
        _cursorBlinkTimer   = 0.f;
        _cursorBlinkVisible = true;
        return;
    }

    // Click elsewhere deactivates text input
    _nameInputActive = false;

    // Left column: games
    if (relativeX < MenuLayout::LEFT_ZONE_BOUNDARY) {
        for (int i = 0; i < (int)_gameLibs.size(); ++i) {
            float itemY = MenuLayout::GAMES_START_Y + i * MenuLayout::GAMES_STEP_Y;
            if (std::abs(relativeY - itemY) < MenuLayout::GAMES_STEP_Y * 0.5f) {
                _menuGameCursor = i;
                _currentGame = i;
                loadGame(_currentGame);
                _state = State::PLAYING;
                return;
            }
        }
    }

    // Right column: graphics
    if (relativeX > MenuLayout::RIGHT_ZONE_BOUNDARY) {
        for (int i = 0; i < (int)_displayLibs.size(); ++i) {
            float itemY = MenuLayout::GFX_START_Y + i * MenuLayout::GFX_STEP_Y;
            if (std::abs(relativeY - itemY) < MenuLayout::GFX_STEP_Y * 0.5f) {
                _menuGraphicsCursor = i;
                _currentDisplay = i;
                loadDisplay(_currentDisplay);
                return;
            }
        }
    }
}

void Core::renderLoading(const std::string& targetName)
{
    if (!_display)
        return;

    _display->clear();
    _display->drawText("Loading " + targetName + "...",
                       MenuLayout::LOADING_X, MenuLayout::LOADING_Y,
                       MenuLayout::LOADING_COLOR, nullptr,
                       MenuLayout::LOADING_SCALE);
    _display->display();
    std::this_thread::sleep_for(std::chrono::milliseconds(CoreConfig::LOADING_DELAY_MS));
}

void Core::renderMenu()
{
    _display->clear();

    // ── Title ─────────────────────────────────────────────────────────────────
    _display->drawText("* A R C A D E *",
                       MenuLayout::TITLE_X, MenuLayout::TITLE_Y,
                       MenuLayout::TITLE_COLOR, nullptr,
                       MenuLayout::TITLE_SCALE);

    // ── Menu Title Bar ────────────────────────────────────────────────────────
    _display->drawLine(MenuLayout::MENU_LINE_CHAR,
        MenuLayout::MENU_LINE_X_START, MenuLayout::MENU_LINE_X_END,
        MenuLayout::MENU_LINE_Y,
        MenuLayout::MENU_LINE_COLOR, MenuLayout::MENU_LINE_BG_COLOR);

    // ── Player name ───────────────────────────────────────────────────────────
    const Color nameColor = _nameInputActive
        ? MenuLayout::PLAYER_NAME_ACTIVE_COLOR
        : (_nameInputHovered ? MenuLayout::PLAYER_NAME_HOVER_COLOR : MenuLayout::PLAYER_NAME_COLOR);
    const std::string cursor = (_nameInputActive && _cursorBlinkVisible)
        ? std::string(1, MenuLayout::PLAYER_NAME_CURSOR_CHAR) : "";
    
    const std::string prefix = (cursor != "") ? " " : "";
    const std::string textToDraw = prefix + "Enter name: " + _playerName + cursor;
    _display->drawText(textToDraw,
                        MenuLayout::PLAYER_NAME_X, MenuLayout::PLAYER_NAME_Y,
                        nameColor, nullptr,
                        MenuLayout::PLAYER_NAME_SCALE);

    // ── Left column: Games ────────────────────────────────────────────────────
    _display->drawText("[ GAMES ]",
                       MenuLayout::GAMES_HDR_X, MenuLayout::GAMES_HDR_Y,
                       MenuLayout::GAMES_HDR_COLOR, nullptr,
                       MenuLayout::GAMES_HDR_SCALE);

    for (int i = 0; i < (int)_gameLibs.size(); ++i) {
        const bool sel = (_menuGameCursor >= 0 && i == _menuGameCursor);
        const float y = MenuLayout::GAMES_START_Y + i * MenuLayout::GAMES_STEP_Y;
        _display->drawText(
            sel ? ">> " + _gameLibs[i].name + " <<" : _gameLibs[i].name,
            MenuLayout::GAMES_ITEM_X, y,
            sel ? MenuLayout::GAMES_SELECTED_COLOR : MenuLayout::GAMES_COLOR);
    }

    // ── Center column: Scores ─────────────────────────────────────────────────
    const std::string scoresGameName =
        (!_gameLibs.empty() && _menuGameCursor >= 0 && _menuGameCursor < (int)_gameLibs.size())
        ? _gameLibs[_menuGameCursor].name : "";

    _display->drawText("[ TOP SCORES" + (scoresGameName.empty() ? "" : (" - " + scoresGameName)) + " ]",
                       MenuLayout::SCORES_HDR_X, MenuLayout::SCORES_HDR_Y,
                       MenuLayout::SCORES_HDR_COLOR, nullptr,
                       MenuLayout::SCORES_HDR_SCALE);

    if (!scoresGameName.empty()) {
        auto it = _scores.find(scoresGameName);
        if (it != _scores.end() && !it->second.empty()) {
            for (int i = 0; i < (int)it->second.size(); ++i) {
                const float y = MenuLayout::SCORES_START_Y + i * MenuLayout::SCORES_STEP_Y;
                const auto& e = it->second[i];
                const std::string entry =
                    std::to_string(i + 1) + ". " +
                    e.playerName + " - " +
                    std::to_string(e.score) + " points - " +
                    formatDuration(e.durationSeconds) + " - " +
                    e.date;
                _display->drawText(entry, MenuLayout::SCORES_ITEM_X, y,
                                   MenuLayout::SCORES_COLOR);
            }
        } else {
            _display->drawText("No scores yet",
                               MenuLayout::SCORES_ITEM_X, MenuLayout::SCORES_START_Y,
                               MenuLayout::SCORES_NAME_COLOR);
        }
    }

    // ── Right column: Graphics ────────────────────────────────────────────────
    _display->drawText("[ GRAPHICS ]",
                       MenuLayout::GFX_HDR_X, MenuLayout::GFX_HDR_Y,
                       MenuLayout::GFX_HDR_COLOR, nullptr,
                       MenuLayout::GFX_HDR_SCALE);

    for (int i = 0; i < (int)_displayLibs.size(); ++i) {
        const bool sel = (_menuGraphicsCursor >= 0 && i == _menuGraphicsCursor);
        const bool active = (i == _currentDisplay);
        const std::string label = (active ? "[X] " : "[ ] ") + _displayLibs[i].name;
        const float y = MenuLayout::GFX_START_Y + i * MenuLayout::GFX_STEP_Y;
        _display->drawText(
            sel ? ">> " + label + " <<" : label,
            MenuLayout::GFX_ITEM_X, y,
            sel ? MenuLayout::GFX_SELECTED_COLOR
                : (active ? MenuLayout::GFX_ACTIVE_COLOR : MenuLayout::GFX_COLOR));
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    _display->drawText("ARROWS: Navigate   ENTER: Play   MOUSE: Click   Click name to type",
                       MenuLayout::FOOTER1_X, MenuLayout::FOOTER1_Y,
                       MenuLayout::FOOTER1_COLOR);
    _display->drawText("TAB: Change Graphics   ESC: Exit   +/-: Volume   BACKSPACE: Edit Name",
                       MenuLayout::FOOTER2_X, MenuLayout::FOOTER2_Y,
                       MenuLayout::FOOTER2_COLOR);

    renderVolumeHint();
    _display->display();
}

} // namespace Arcade
