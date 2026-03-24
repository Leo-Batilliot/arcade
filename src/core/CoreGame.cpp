#include "Core.hpp"
#include <algorithm>
#include <chrono>

namespace Arcade {
    extern const unsigned char LOSING_OGG_DATA[];
    extern const unsigned int  LOSING_OGG_SIZE;
    extern const unsigned char WIN_OGG_DATA[];
    extern const unsigned int  WIN_OGG_SIZE;
}

namespace Arcade {

void Core::runGame()
{
    using Clock = std::chrono::steady_clock;
    auto prev = Clock::now();

    while (_state == State::PLAYING && _display && _display->isOpen()) {
        auto now = Clock::now();
        float deltaTime = std::chrono::duration<float>(now - prev).count();
        prev = now;
        if (deltaTime > CoreConfig::MAX_DELTA_TIME)
            deltaTime = CoreConfig::MAX_DELTA_TIME;

        const auto rawKeys     = _display->getKeys();
        const auto heldKeys    = _display->getHeldKeys();
        const auto mouseEvents = _display->getMouseEvents();
        _display->getTextInput(); // drain — prevent accumulation into menu player name

        // Merge raw events + hardware-polled held keys for game movement.
        // ncurses: heldKeys is empty → rawKeys only (OS key-repeat drives movement).
        // SFML/SDL2: union of both → diagonal movement + no startup delay.
        std::vector<Key> gameKeys = rawKeys;
        for (const auto k : heldKeys) {
            if (std::find(gameKeys.begin(), gameKeys.end(), k) == gameKeys.end())
                gameKeys.push_back(k);
        }

        handleVolumeKeys(rawKeys);

        for (const auto key : rawKeys) {
            if (key == Key::TAB) {
                const int nextIndex = (_currentDisplay + 1) % _displayLibs.size();
                renderLoading(_displayLibs[nextIndex].name);
                nextDisplay();
                return;
            }
            if (key == Key::ESCAPE) {
                if (_game && _game->getScore() > 0)
                    updateScore(_gameLibs[_currentGame].name, _game->getScore());
                _state = State::MENU;
                unloadGame();
                startMenuMusic();
                return;
            }
            if (key == Key::R) {
                restartGame();
                return;
            }
            if (key == Key::G && _gameLibs.size() > 1) {
                const int nextIndex = (_currentGame + 1) % (int)_gameLibs.size();
                renderLoading(_gameLibs[nextIndex].name);
                nextGame();
                return;
            }
        }

        if (_game) {
            for (const auto& mouseEvent : mouseEvents)
                _game->onMouseEvent(mouseEvent);

            if (_display->hasMouseMoved()) {
                auto [relativeX, relativeY] = _display->getMousePosition();
                if (relativeX >= 0.f) {
                    MouseEvent hover;
                    hover.relativeX = relativeX;
                    hover.relativeY = relativeY;
                    hover.gridX = -1;
                    hover.gridY = -1;
                    _game->onMouseEvent(hover);
                }
            }

            _game->update(deltaTime, gameKeys);
            _display->clear();
            _game->render(*_display);
            renderHUD();
            _display->display();

            if (_game->isGameOver() || _game->hasWon()) {
                _display->stopMusic();
                _isNewRecord = updateScore(_gameLibs[_currentGame].name, _game->getScore());
                if (_isNewRecord && WIN_OGG_SIZE > 0)
                    _display->playSound(WIN_OGG_DATA, WIN_OGG_SIZE);
                else
                    _display->playSound(LOSING_OGG_DATA, LOSING_OGG_SIZE);
                _state = State::GAMEOVER;
                return;
            }
        }
    }
}

void Core::renderHUD()
{
    if (!_display || !_game)
        return;

    _display->drawLine(CoreConfig::HUD_LINE_CHAR,
        CoreConfig::HUD_LINE_X_START, CoreConfig::HUD_LINE_X_END,
        CoreConfig::HUD_LINE_Y,
        CoreConfig::HUD_LINE_COLOR, CoreConfig::HUD_LINE_BG_COLOR);

    const int nextDisplayIndex = (_currentDisplay + 1) % _displayLibs.size();
    const std::string tabText =
        "TAB: Cycle Graphical interface [" +
        _displayLibs[_currentDisplay].name + " -> " +
        _displayLibs[nextDisplayIndex].name + "]";
    _display->drawText(tabText,
        CoreConfig::HUD_TAB_TEXT_X, CoreConfig::HUD_TAB_TEXT_Y,
        CoreConfig::HUD_TAB_TEXT_COLOR, &CoreConfig::HUD_TAB_TEXT_BG_COLOR,
        CoreConfig::HUD_TAB_TEXT_SCALE);

    const std::string keysText = "ESC: Menu   R: Restart   G: Next Game";
    _display->drawText(keysText,
        CoreConfig::HUD_KEYS_TEXT_X, CoreConfig::HUD_KEYS_TEXT_Y,
        CoreConfig::HUD_KEYS_TEXT_COLOR, &CoreConfig::HUD_KEYS_TEXT_BG_COLOR,
        CoreConfig::HUD_KEYS_TEXT_SCALE);

    renderVolumeHint();
}

} // namespace Arcade
