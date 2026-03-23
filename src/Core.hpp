#pragma once
#include "CoreConfig.hpp"
#include "DLLoader.hpp"
#include "IDisplayModule.hpp"
#include "IGameModule.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Arcade {

/// @brief Central controller that owns the display and game modules and drives
///        the main loop, library scanning, and state transitions.
class Core {
public:
    /// @brief Constructs the Core and loads the initial display library.
    /// @param initialDisplayLib Path to the shared library to open first.
    explicit Core(const std::string& initialDisplayLib);

    /// @brief Destroys the Core, unloading any active game and display modules.
    ~Core();

    /// @brief Enters the main event/render loop and runs until the user quits.
    void run();

    Core(const Core&)            = delete;
    Core& operator=(const Core&) = delete;

private:
    enum class State { MENU, PLAYING, GAMEOVER };
    State _state   = State::MENU;
    bool  _running = true;
    int   _volume  = 100;

    std::string _playerName = "Player";

    struct ScoreEntry {
        std::string playerName;
        int score = 0;
    };
    std::map<std::string, std::vector<ScoreEntry>> _scores;

    struct LibInfo {
        std::string path;
        std::string name;
    };
    std::vector<LibInfo> _displayLibs;
    std::vector<LibInfo> _gameLibs;
    int _currentDisplay     = 0;
    int _currentGame        = 0;
    int _menuGameCursor     = 0;
    int _menuGraphicsCursor = 0;

    using DestroyDisplayFn = void(*)(IDisplayModule*);
    std::unique_ptr<DLLoader> _displayLoader;
    IDisplayModule*           _display        = nullptr;
    DestroyDisplayFn          _destroyDisplay = nullptr;

    using DestroyGameFn = void(*)(IGameModule*);
    std::unique_ptr<DLLoader>  _gameLoader;
    IGameModule*               _game        = nullptr;
    DestroyGameFn              _destroyGame = nullptr;

    /// @brief Owned copy of the current game's music data; empty when no game is loaded.
    std::vector<unsigned char> _musicBuffer;

    /// @brief Scans the lib/ directory and populates _displayLibs and _gameLibs.
    void scanLibraries();

    /// @brief Loads the display library at the given index and opens a window.
    /// @param index Index into _displayLibs.
    void loadDisplay(int index);

    /// @brief Unloads the currently active display module.
    void unloadDisplay();

    /// @brief Loads the game library at the given index and initialises it.
    /// @param index Index into _gameLibs.
    void loadGame(int index);

    /// @brief Unloads the currently active game module.
    void unloadGame();

    /// @brief Switches to the next display library in the list, wrapping around.
    void nextDisplay();

    /// @brief Switches to the previous display library in the list, wrapping around.
    void prevDisplay();

    /// @brief Switches to the next game in the list, reloading in-place.
    void nextGame();

    /// @brief Restarts the current game (close + init) without unloading it.
    void restartGame();

    /// @brief Load high scores from scores.txt.
    void loadScores();

    /// @brief Persist high scores to scores.txt.
    void saveScores();

    /// @brief Update the stored high score for @p gameName if @p score is better.
    void updateScore(const std::string& gameName, int score);

    void runMenu();
    void runGame();
    void runGameOver();

    void renderMenu();
    void renderHUD();
    void renderGameOverScreen();
    void renderLoading(const std::string& targetName);

    void handleMenuMouseHover();
    void handleMenuMouseClick(float relativeX, float relativeY);

    /// @brief Process volume keys (+/-) from any state; applies change immediately.
    void handleVolumeKeys(const std::vector<Key>& keys);

    /// @brief Draw the persistent volume indicator in the top-right corner.
    void renderVolumeHint();

    /// @brief (Re)start the music stored in _musicBuffer on the active display.
    void playCurrentMusic();

    /// @brief Load menu music into _musicBuffer and start playing it.
    void startMenuMusic();
};

} // namespace Arcade
