#include "Minesweeper.hpp"
#include "IDisplayModule.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

namespace Arcade {
    extern const unsigned char MINESWEEPER_OGG_DATA[];
    extern const unsigned int  MINESWEEPER_OGG_SIZE;
    extern const unsigned char FLAG_OGG_DATA[];
    extern const unsigned int  FLAG_OGG_SIZE;
    extern const unsigned char SAFE_CELL_OGG_DATA[];
    extern const unsigned int  SAFE_CELL_OGG_SIZE;
    extern const unsigned char BOMB_OGG_DATA[];
    extern const unsigned int  BOMB_OGG_SIZE;
    extern const unsigned char NEXT_LEVEL_OGG_DATA[];
    extern const unsigned int  NEXT_LEVEL_OGG_SIZE;
    extern const unsigned char FLAG_JPG_DATA[];
    extern const unsigned int  FLAG_JPG_SIZE;
    extern const unsigned char BOMB_PNG_DATA[];
    extern const unsigned int  BOMB_PNG_SIZE;
    extern const unsigned char UNKNOWN_CELL_PNG_DATA[];
    extern const unsigned int  UNKNOWN_CELL_PNG_SIZE;
}

namespace Arcade {

const unsigned char* Minesweeper::getMusicData() const { return MINESWEEPER_OGG_DATA; }
unsigned int         Minesweeper::getMusicSize()  const { return MINESWEEPER_OGG_SIZE; }

int Minesweeper::minesForSize(int cols, int rows) noexcept
{
    return std::max(1, (int)std::round(cols * rows * MinesweeperConfig::MINE_RATIO));
}

int Minesweeper::nextOddSize(int current) noexcept
{
    int next = (int)std::ceil(current * MinesweeperConfig::LEVEL_GROWTH);
    if (next % 2 == 0)
        ++next;
    return next;
}

void Minesweeper::init()
{
    _cols  = MinesweeperConfig::INITIAL_SIZE;
    _rows  = MinesweeperConfig::INITIAL_SIZE;
    _level = 1;
    _score = 0;
    initBoard();
}

void Minesweeper::initBoard()
{
    _mines    = minesForSize(_cols, _rows);
    _gameOver = false;
    _started  = false;
    _elapsed  = 0.f;
    _currentX = _cols / 2;
    _currentY = _rows / 2;

    _soundFlag      = false;
    _soundSafe      = false;
    _soundBomb      = false;
    _soundNextLevel = false;

    _board.assign(_rows, std::vector<BoardCell>(_cols));
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Minesweeper::placeMines(int safeX, int safeY)
{
    int placed = 0;
    while (placed < _mines) {
        int x = std::rand() % _cols;
        int y = std::rand() % _rows;
        if (_board[y][x].mine)
            continue;
        if (std::abs(x - safeX) <= 1 && std::abs(y - safeY) <= 1)
            continue;
        _board[y][x].mine = true;
        ++placed;
    }
    computeAdjacent();
}

void Minesweeper::computeAdjacent()
{
    for (int y = 0; y < _rows; ++y) {
        for (int x = 0; x < _cols; ++x) {
            if (_board[y][x].mine)
                continue;
            int count = 0;
            for (int deltaY = -1; deltaY <= 1; ++deltaY) {
                for (int deltaX = -1; deltaX <= 1; ++deltaX) {
                    if (!deltaX && !deltaY)
                        continue;
                    const int neighborX = x + deltaX;
                    const int neighborY = y + deltaY;
                    if (neighborX >= 0 && neighborX < _cols &&
                        neighborY >= 0 && neighborY < _rows &&
                        _board[neighborY][neighborX].mine)
                        ++count;
                }
            }
            _board[y][x].adjacent = count;
        }
    }
}

bool Minesweeper::reveal(int x, int y)
{
    if (x < 0 || x >= _cols || y < 0 || y >= _rows)
        return false;
    auto& boardCell = _board[y][x];
    if (boardCell.revealed || boardCell.flagged)
        return false;
    boardCell.revealed = true;
    if (boardCell.mine) {
        _gameOver    = true;
        _soundBomb   = true;
        for (auto& row : _board)
            for (auto& cell : row)
                if (cell.mine)
                    cell.revealed = true;
        return false;
    }
    _score += MinesweeperConfig::SCORE_PER_REVEAL;
    if (boardCell.adjacent == 0) {
        for (int deltaY = -1; deltaY <= 1; ++deltaY)
            for (int deltaX = -1; deltaX <= 1; ++deltaX)
                if (deltaX || deltaY)
                    reveal(x + deltaX, y + deltaY);
    }
    return true;
}

void Minesweeper::checkWin()
{
    for (auto& row : _board)
        for (auto& boardCell : row)
            if (!boardCell.mine && !boardCell.revealed)
                return;

    // Level cleared — advance to the next, larger level
    _soundNextLevel = true;
    _score += MinesweeperConfig::SCORE_PER_REVEAL * _cols * _rows; // bonus
    ++_level;
    _cols = nextOddSize(_cols);
    _rows = _cols; // keep square
    initBoard();
}

int Minesweeper::flagCount() const noexcept
{
    int count = 0;
    for (auto& row : _board)
        for (auto& boardCell : row)
            count += boardCell.flagged;
    return count;
}

void Minesweeper::onMouseEvent(const MouseEvent& event)
{
    if (event.gridX < 0 || event.gridX >= _cols ||
        event.gridY < 0 || event.gridY >= _rows)
        return;
    _currentX = event.gridX;
    _currentY = event.gridY;

    if (event.button == Key::MOUSE_LEFT) {
        if (!_started) {
            _started = true;
            placeMines(_currentX, _currentY);
        }
        const bool safe = reveal(_currentX, _currentY);
        if (safe)
            _soundSafe = true;
        if (!_gameOver)
            checkWin();
    } else if (event.button == Key::MOUSE_RIGHT) {
        auto& boardCell = _board[_currentY][_currentX];
        if (!boardCell.revealed) {
            boardCell.flagged = !boardCell.flagged;
            _soundFlag = true;
        }
    }
}

void Minesweeper::update(float deltaTime, const std::vector<Key>& keys)
{
    if (_gameOver)
        return;
    _totalTime += deltaTime;
    if (_started)
        _elapsed += deltaTime;

    for (auto& pair : _keyStates) {
        if (std::find(keys.begin(), keys.end(), pair.first) == keys.end()) {
            pair.second.isPressed = false;
        }
    }

    for (const auto key : keys) {
        auto& state = _keyStates[key]; 

        bool executeAction = false;

        if (!state.isPressed) {
            state.isPressed = true;
            state.lastActionTime = _totalTime;
            executeAction = true;
        } else if (_totalTime - state.lastActionTime >= MinesweeperConfig::KEY_REPEAT_DELAY) {
            state.lastActionTime = _totalTime;
            executeAction = true;
        }
        if (!executeAction)
            continue;
        if (key == Key::UP    && _currentY > 0)          --_currentY;
        if (key == Key::DOWN  && _currentY < _rows - 1)  ++_currentY; 
        if (key == Key::LEFT  && _currentX > 0)          --_currentX;
        if (key == Key::RIGHT && _currentX < _cols - 1)  ++_currentX;
        if (key == Key::ENTER) {
            if (!_started) {
                _started = true;
                placeMines(_currentX, _currentY);
            }
            const bool safe = reveal(_currentX, _currentY);
            if (safe)
                _soundSafe = true;
            if (!_gameOver)
                checkWin();
        }
        if (key == Key::SPACE) {
            auto& boardCell = _board[_currentY][_currentX];
            if (!boardCell.revealed) {
                boardCell.flagged = !boardCell.flagged;
                _soundFlag = true;
            }
        }
        if (key == Key::R) {
            init();
            return;
        }
    }
}

Color Minesweeper::adjacentColor(unsigned char count) noexcept
{
    switch (count) {
        case 1:  return Colors::Blue;
        case 2:  return Colors::Green;
        case 3:  return Colors::Yellow;
        case 4:  return Colors::Red;
        default: return Colors::Red;
    }
}

void Minesweeper::render(IDisplayModule& display)
{
    // ── Consume sound events ──────────────────────────────────────────────────
    if (_soundBomb) {
        display.playSound(BOMB_OGG_DATA, BOMB_OGG_SIZE);
        _soundBomb = false;
    } else if (_soundNextLevel) {
        display.playSound(NEXT_LEVEL_OGG_DATA, NEXT_LEVEL_OGG_SIZE);
        _soundNextLevel = false;
    } else if (_soundFlag) {
        display.playSound(FLAG_OGG_DATA, FLAG_OGG_SIZE);
        _soundFlag = false;
    } else if (_soundSafe) {
        display.playSound(SAFE_CELL_OGG_DATA, SAFE_CELL_OGG_SIZE);
        _soundSafe = false;
    }

    // ── Build cell grid ───────────────────────────────────────────────────────
    static const Color cellBackground = Colors::DarkGray;

    std::vector<Cell> cells(_cols * _rows);
    for (int y = 0; y < _rows; ++y) {
        for (int x = 0; x < _cols; ++x) {
            const auto& boardCell = _board[y][x];
            const bool  isCurrent = (x == _currentX && y == _currentY);

            Cell cell;
            cell.highlight = isCurrent;

            if (!boardCell.revealed) {
                if (boardCell.flagged) {
                    cell.c       = 'F';
                    cell.fg      = Colors::Red;
                    cell.bg      = cellBackground;
                    cell.bgImage = cellBackground;
                    if (FLAG_JPG_SIZE > 0) {
                        cell.imageData = FLAG_JPG_DATA;
                        cell.imageSize = FLAG_JPG_SIZE;
                    }
                } else {
                    cell.c       = '.';
                    cell.fg      = Colors::White;
                    cell.bg      = cellBackground;
                    cell.bgImage = cellBackground;
                    if (UNKNOWN_CELL_PNG_SIZE > 0) {
                        cell.imageData = UNKNOWN_CELL_PNG_DATA;
                        cell.imageSize = UNKNOWN_CELL_PNG_SIZE;
                    }
                }
            } else if (boardCell.mine) {
                cell.c       = '*';
                cell.fg      = Colors::Yellow;
                cell.bg      = Colors::DarkRed;
                cell.bgImage = Colors::DarkRed;
                if (BOMB_PNG_SIZE > 0) {
                    cell.imageData = BOMB_PNG_DATA;
                    cell.imageSize = BOMB_PNG_SIZE;
                }
            } else if (boardCell.adjacent == 0) {
                cell.c  = ' ';
                cell.fg = Colors::White;
                cell.bg = Color{40, 40, 40, 255};
            } else {
                cell.c  = '0' + (char)boardCell.adjacent;
                cell.fg = adjacentColor((unsigned char)boardCell.adjacent);
                cell.bg = Color{40, 40, 40, 255};
            }

            cells[y * _cols + x] = cell;
        }
    }

    display.drawText("MINESWEEPER  LVL " + std::to_string(_level),
                     MinesweeperLayout::TITLE_X, MinesweeperLayout::TITLE_Y,
                     MinesweeperLayout::TITLE_COLOR, nullptr,
                     MinesweeperLayout::TITLE_SCALE);
    const std::string stats =
        "Mines left:" + std::to_string(_mines - flagCount()) +
        "     Time:" + std::to_string((int)_elapsed) + "s" +
        "     Score:" + std::to_string(_score);
    display.drawText(stats,
                     MinesweeperLayout::STATS_X, MinesweeperLayout::STATS_Y,
                     MinesweeperLayout::STATS_COLOR, nullptr,
                     MinesweeperLayout::STATS_SCALE);

    display.drawGrid(cells, _cols, _rows,
                     MinesweeperConfig::ZONE_X1, MinesweeperConfig::ZONE_Y1,
                     MinesweeperConfig::ZONE_X2, MinesweeperConfig::ZONE_Y2,
                     _currentX, _currentY,
                     MinesweeperConfig::GRID_STYLE);

    display.drawText("ENTER:Reveal      SPC:Flag",
                     MinesweeperLayout::CONTROLS_X, MinesweeperLayout::CONTROLS_Y,
                     MinesweeperLayout::CONTROLS_COLOR, nullptr,
                     MinesweeperLayout::CONTROLS_SCALE);
}

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame() { return new Arcade::Minesweeper(); }
    void destroyGame(Arcade::IGameModule* game) { delete game; }
}
