#include "Core.hpp"
#include <chrono>
#include <thread>

namespace Arcade {

void Core::runGameOver()
{
    while (_state == State::GAMEOVER && _display && _display->isOpen()) {
        const auto keys = _display->getKeys();
        const auto mouseEvents = _display->getMouseEvents();

        handleVolumeKeys(keys);

        bool hasAction = false;
        for (const auto key : keys) {
            if (key != Key::PLUS && key != Key::MINUS) {
                hasAction = true;
                break;
            }
        }
        if (!hasAction) {
            for (const auto& mouseEvent : mouseEvents) {
                if (mouseEvent.button != Key::NONE) {
                    hasAction = true;
                    break;
                }
            }
        }

        if (hasAction) {
            _state = State::MENU;
            unloadGame();
            startMenuMusic();
            return;
        }

        renderGameOverScreen();
        std::this_thread::sleep_for(std::chrono::milliseconds(CoreConfig::GAMEOVER_FRAME_MS));
    }
}

void Core::renderGameOverScreen()
{
    if (!_display)
        return;

    _display->clear();

    // Render the final game state as background
    if (_game)
        _game->render(*_display);

    const bool won = _game && _game->hasWon();
    const int score = _game ? _game->getScore() : 0;
    const std::string gameName = _game ? _game->getName() : "?";
    const Color boxColor = won ? GameOverLayout::BOX_WON_COLOR
                               : GameOverLayout::BOX_LOST_COLOR;

    // Semi-transparent dark overlay behind the game-over box
    _display->drawRect(GameOverLayout::BOX_LINE_X1, GameOverLayout::BOX_TOP_LINE_Y,
                       GameOverLayout::BOX_LINE_X2, GameOverLayout::BOX_BOT_LINE_Y,
                       GameOverLayout::OVERLAY_COLOR);

    _display->drawLine(GameOverLayout::BOX_LINE_CHAR,
                       GameOverLayout::BOX_LINE_X1, GameOverLayout::BOX_LINE_X2,
                       GameOverLayout::BOX_TOP_LINE_Y,
                       boxColor, Colors::Black);
    _display->drawLine(GameOverLayout::BOX_LINE_CHAR,
                       GameOverLayout::BOX_LINE_X1, GameOverLayout::BOX_LINE_X2,
                       GameOverLayout::BOX_BOT_LINE_Y,
                       boxColor, Colors::Black);

    _display->drawText(won ? "CONGRATULATIONS!" : "GAME  OVER",
                       GameOverLayout::TITLE_X, GameOverLayout::TITLE_Y,
                       boxColor, nullptr,
                       GameOverLayout::TITLE_SCALE);

    _display->drawText(gameName,
                       GameOverLayout::GAME_NAME_X, GameOverLayout::GAME_NAME_Y,
                       GameOverLayout::GAME_NAME_COLOR, nullptr,
                       GameOverLayout::GAME_NAME_SCALE);

    _display->drawText("Score : " + std::to_string(score),
                       GameOverLayout::SCORE_X, GameOverLayout::SCORE_Y,
                       GameOverLayout::SCORE_COLOR, nullptr,
                       GameOverLayout::SCORE_SCALE);

    _display->drawText("Press any key to continue...",
                       GameOverLayout::CONTINUE_X, GameOverLayout::CONTINUE_Y,
                       GameOverLayout::CONTINUE_COLOR, nullptr,
                       GameOverLayout::CONTINUE_SCALE);

    renderVolumeHint();
    _display->display();
}

} // namespace Arcade
