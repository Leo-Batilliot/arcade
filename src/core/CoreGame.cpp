#include "Core.hpp"
#include <chrono>

namespace Arcade {
    extern const unsigned char LOSING_OGG_DATA[];
    extern const unsigned int  LOSING_OGG_SIZE;
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

        const auto keys        = _display->getKeys();
        const auto mouseEvents = _display->getMouseEvents();
        _display->getTextInput(); // drain — prevent accumulation into menu player name

        handleVolumeKeys(keys);

        for (const auto key : keys) {
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

            _game->update(deltaTime, keys);
            _display->clear();
            _game->render(*_display);
            renderHUD();
            _display->display();

            if (_game->isGameOver() || _game->hasWon()) {
                _display->stopMusic();
                _display->playSound(LOSING_OGG_DATA, LOSING_OGG_SIZE);
                updateScore(_gameLibs[_currentGame].name, _game->getScore());
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

    const std::string scoreText =
        _game->getName() + "   Score: " + std::to_string(_game->getScore());
    _display->drawText(scoreText,
        CoreConfig::HUD_SCORE_TEXT_X, CoreConfig::HUD_SCORE_TEXT_Y,
        CoreConfig::HUD_SCORE_TEXT_COLOR, nullptr,
        CoreConfig::HUD_SCORE_TEXT_SCALE);

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
