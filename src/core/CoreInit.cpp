#include "Core.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace Arcade {

Core::Core(const std::string& initialDisplayLib)
{
    scanLibraries();
    if (_displayLibs.empty())
        throw std::runtime_error("No display library found in ./lib/");

    const auto stem = fs::path(initialDisplayLib).stem().string();
    int index = 0;

    for (int i = 0; i < (int)_displayLibs.size(); ++i) {
        if (fs::path(_displayLibs[i].path).stem().string() == stem ||
            _displayLibs[i].path == initialDisplayLib) {
            index = i;
            break;
        }
    }

    if (fs::path(_displayLibs[index].path).stem().string() != stem) {
        std::string name         = stem;
        const auto underscorePos = stem.find('_');
        if (underscorePos != std::string::npos)
            name = stem.substr(underscorePos + 1);
        _displayLibs.insert(_displayLibs.begin(), {initialDisplayLib, name});
        index = 0;
    }

    _currentDisplay = index;
    loadScores();
    loadDisplay(_currentDisplay);
    startMenuMusic();
}

Core::~Core()
{
    unloadGame();
    unloadDisplay();
}

void Core::scanLibraries()
{
    if (!fs::exists(CoreConfig::LIB_DIR))
        return;

    for (const auto& entry : fs::directory_iterator(CoreConfig::LIB_DIR)) {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() != CoreConfig::LIB_EXT)
            continue;
        const std::string stem = entry.path().stem().string();
        if (stem.rfind(CoreConfig::LIB_PREFIX, 0) != 0)
            continue;
        const std::string path = entry.path().string();
        const std::string name = stem.substr(std::string(CoreConfig::LIB_PREFIX).size());
        try {
            DLLoader loader(path);
            if (loader.hasSymbol("createDisplay"))
                _displayLibs.push_back({path, name});
            else if (loader.hasSymbol("createGame"))
                _gameLibs.push_back({path, name});
        } catch (...) {}
    }

    auto byName = [](const LibInfo& lhs, const LibInfo& rhs){ return lhs.name < rhs.name; };
    std::sort(_displayLibs.begin(), _displayLibs.end(), byName);
    std::sort(_gameLibs.begin(),    _gameLibs.end(),    byName);
}

void Core::loadDisplay(int index)
{
    if (_displayLibs.empty())
        return;

    auto newLoader  = std::make_unique<DLLoader>(_displayLibs[index].path);
    auto create     = newLoader->getSymbol<IDisplayModule*(*)()>("createDisplay");
    auto newDestroy = newLoader->getSymbol<DestroyDisplayFn>("destroyDisplay");
    IDisplayModule* newDisplay = create();

    if (_display && _destroyDisplay) {
        _display->close();
        _destroyDisplay(_display);
    }
    _displayLoader.reset();

    _display        = newDisplay;
    _destroyDisplay = newDestroy;
    _displayLoader  = std::move(newLoader);
    _display->init("Arcade — " + _displayLibs[index].name);
    _display->setVolume(_volume);
    playCurrentMusic();
}

void Core::unloadDisplay()
{
    if (_display && _destroyDisplay) {
        _display->close();
        _destroyDisplay(_display);
    }

    _display        = nullptr;
    _destroyDisplay = nullptr;
    _displayLoader.reset();
}

void Core::loadGame(int index)
{
    if (_gameLibs.empty() || !_display)
        return;

    unloadGame();
    _gameLoader  = std::make_unique<DLLoader>(_gameLibs[index].path);
    auto create  = _gameLoader->getSymbol<IGameModule*(*)()>("createGame");
    _destroyGame = _gameLoader->getSymbol<DestroyGameFn>("destroyGame");
    _game = create();
    _game->init();
    _gameStartTime = std::chrono::steady_clock::now();

    const unsigned char* musicData = _game->getMusicData();
    const unsigned int   musicSize = _game->getMusicSize();
    if (musicData && musicSize > 0)
        _musicBuffer.assign(musicData, musicData + musicSize);
    else
        _musicBuffer.clear();
    playCurrentMusic();
}

void Core::unloadGame()
{
    if (_display) {
        _display->stopMusic();
        _display->clearImageCache();
    }
    _musicBuffer.clear();

    if (_game && _destroyGame) {
        _game->close();
        _destroyGame(_game);
    }

    _game        = nullptr;
    _destroyGame = nullptr;
    _gameLoader.reset();
}

void Core::nextDisplay()
{
    if (_displayLibs.empty())
        return;
    _currentDisplay = (_currentDisplay + 1) % (int)_displayLibs.size();
    loadDisplay(_currentDisplay);
}

void Core::prevDisplay()
{
    if (_displayLibs.empty())
        return;
    _currentDisplay = (_currentDisplay + (int)_displayLibs.size() - 1)
                    % (int)_displayLibs.size();
    loadDisplay(_currentDisplay);
}

void Core::nextGame()
{
    if (_gameLibs.empty())
        return;
    _currentGame    = (_currentGame + 1) % (int)_gameLibs.size();
    _menuGameCursor = _currentGame;
    loadGame(_currentGame);
}

void Core::restartGame()
{
    if (!_game)
        return;
    _game->close();
    _game->init();
    _gameStartTime = std::chrono::steady_clock::now();
    if (!_musicBuffer.empty())
        playCurrentMusic();
}

static std::string currentDate()
{
    const std::time_t t = std::time(nullptr);
    const std::tm tm    = *std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%d/%m/%Y", &tm);
    return buf;
}

void Core::loadScores()
{
    std::ifstream file(CoreConfig::SCORES_FILE);
    if (!file.is_open())
        return;
    std::string line;
    while (std::getline(file, line)) {
        std::string gameName;
        std::string playerName;
        std::string scoreStr;
        std::string durationStr;
        std::string date;
        std::istringstream lineStream(line);
        if (!std::getline(lineStream, gameName,    '|')) continue;
        if (!std::getline(lineStream, playerName,  '|')) continue;
        if (!std::getline(lineStream, scoreStr,    '|')) continue;
        if (!std::getline(lineStream, durationStr, '|')) continue;
        if (!std::getline(lineStream, date))              continue;
        _scores[gameName].push_back({playerName, std::stoi(scoreStr),
                                     std::stoi(durationStr), date});
    }
    for (auto& [gameName, entries] : _scores) {
        std::sort(entries.begin(), entries.end(),
            [](const ScoreEntry& lhs, const ScoreEntry& rhs) {
                if (lhs.score != rhs.score) return lhs.score > rhs.score;
                return lhs.durationSeconds < rhs.durationSeconds;
            });
        if (entries.size() > (std::size_t)CoreConfig::MAX_SCORES_PER_GAME)
            entries.resize(CoreConfig::MAX_SCORES_PER_GAME);
    }
}

void Core::saveScores()
{
    std::ofstream file(CoreConfig::SCORES_FILE);
    if (!file.is_open())
        return;
    for (const auto& [gameName, entries] : _scores) {
        for (const auto& entry : entries) {
            file << gameName << "|"
                 << entry.playerName << "|"
                 << entry.score << "|"
                 << entry.durationSeconds << "|"
                 << entry.date << "\n";
        }
    }
}

void Core::updateScore(const std::string& gameName, int score)
{
    const int durationSeconds = (int)std::chrono::duration<float>(
        std::chrono::steady_clock::now() - _gameStartTime).count();
    auto& entries = _scores[gameName];
    entries.push_back({_playerName, score, durationSeconds, currentDate()});
    std::sort(entries.begin(), entries.end(),
        [](const ScoreEntry& lhs, const ScoreEntry& rhs) {
            if (lhs.score != rhs.score) return lhs.score > rhs.score;
            return lhs.durationSeconds < rhs.durationSeconds;
        });
    if (entries.size() > (std::size_t)CoreConfig::MAX_SCORES_PER_GAME)
        entries.resize(CoreConfig::MAX_SCORES_PER_GAME);
    saveScores();
}

void Core::run()
{
    while (_running && _display && _display->isOpen()) {
        if (_state == State::MENU)
            runMenu();
        else if (_state == State::PLAYING)
            runGame();
        else
            runGameOver();
    }
}

} // namespace Arcade
