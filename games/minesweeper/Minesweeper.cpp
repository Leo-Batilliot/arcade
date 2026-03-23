#include "Minesweeper.hpp"
#include "IDisplayModule.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>

namespace Arcade {
    extern const unsigned char MINES_MUSIC_DATA[];
    extern const unsigned int  MINES_MUSIC_SIZE;
    extern const unsigned char MINES_FLAG_DATA[];
    extern const unsigned int  MINES_FLAG_SIZE;
    extern const unsigned char MINES_BOMB_DATA[];
    extern const unsigned int  MINES_BOMB_SIZE;
    extern const unsigned char MINES_UNKNOWN_DATA[];
    extern const unsigned int  MINES_UNKNOWN_SIZE;
}

namespace Arcade {

const unsigned char* Minesweeper::getMusicData() const
{
    return MINES_MUSIC_DATA;
}

unsigned int Minesweeper::getMusicSize() const
{
    return MINES_MUSIC_SIZE;
}

void Minesweeper::init()
{
    _gameOver = false;
    _won      = false;
    _started  = false;
    _score    = 0;
    _elapsed  = 0.f;
    _keyTimer = 0.f;
    _currentX = COLS / 2;
    _currentY = ROWS / 2;

    _board.assign(ROWS, std::vector<BoardCell>(COLS));
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Minesweeper::placeMines(int safeX, int safeY)
{
    int placed = 0;
    while (placed < MINES) {
        int x = std::rand() % COLS;
        int y = std::rand() % ROWS;
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
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            if (_board[y][x].mine)
                continue;
            int count = 0;
            for (int deltaY = -1; deltaY <= 1; ++deltaY) {
                for (int deltaX = -1; deltaX <= 1; ++deltaX) {
                    if (!deltaX && !deltaY)
                        continue;
                    const int neighborX = x + deltaX;
                    const int neighborY = y + deltaY;
                    if (neighborX >= 0 && neighborX < COLS &&
                        neighborY >= 0 && neighborY < ROWS &&
                        _board[neighborY][neighborX].mine)
                        ++count;
                }
            }
            _board[y][x].adjacent = count;
        }
    }
}

void Minesweeper::reveal(int x, int y)
{
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS)
        return;
    auto& boardCell = _board[y][x];
    if (boardCell.revealed || boardCell.flagged)
        return;
    boardCell.revealed = true;
    if (boardCell.mine) {
        _gameOver = true;
        for (auto& row : _board) {
            for (auto& cell : row) {
                if (cell.mine)
                    cell.revealed = true;
            }
        }
        return;
    }
    _score += MinesweeperConfig::SCORE_PER_REVEAL;
    if (boardCell.adjacent == 0) {
        for (int deltaY = -1; deltaY <= 1; ++deltaY) {
            for (int deltaX = -1; deltaX <= 1; ++deltaX) {
                if (deltaX || deltaY)
                    reveal(x + deltaX, y + deltaY);
            }
        }
    }
}

void Minesweeper::checkWin()
{
    for (auto& row : _board) {
        for (auto& boardCell : row) {
            if (!boardCell.mine && !boardCell.revealed)
                return;
        }
    }
    _won = true;
}

int Minesweeper::flagCount() const noexcept
{
    int count = 0;
    for (auto& row : _board) {
        for (auto& boardCell : row)
            count += boardCell.flagged;
    }
    return count;
}

void Minesweeper::onMouseEvent(const MouseEvent& event)
{
    if (event.gridX < 0 || event.gridX >= COLS ||
        event.gridY < 0 || event.gridY >= ROWS)
        return;
    _currentX = event.gridX;
    _currentY = event.gridY;

    if (event.button == Key::MOUSE_LEFT) {
        if (!_started) {
            _started = true;
            placeMines(_currentX, _currentY);
        }
        reveal(_currentX, _currentY);
        if (!_gameOver)
            checkWin();
    } else if (event.button == Key::MOUSE_RIGHT) {
        auto& boardCell = _board[_currentY][_currentX];
        if (!boardCell.revealed)
            boardCell.flagged = !boardCell.flagged;
    }
}

void Minesweeper::update(float deltaTime, const std::vector<Key>& keys)
{
    if (_gameOver || _won)
        return;
    if (_started)
        _elapsed += deltaTime;

    _keyTimer += deltaTime;
    if (_keyTimer < KEY_DELAY)
        return;

    for (const auto key : keys) {
        bool moved = false;

        if (key == Key::UP    && _currentY > 0) { --_currentY; moved = true; }
        if (key == Key::DOWN  && _currentY < ROWS - 1) { ++_currentY; moved = true; }
        if (key == Key::LEFT  && _currentX > 0) { --_currentX; moved = true; }
        if (key == Key::RIGHT && _currentX < COLS - 1) { ++_currentX; moved = true; }
        if (key == Key::ENTER) {
            if (!_started) {
                _started = true;
                placeMines(_currentX, _currentY);
            }
            reveal(_currentX, _currentY);
            if (!_gameOver)
                checkWin();
            moved = true;
        }
        if (key == Key::SPACE) {
            auto& boardCell = _board[_currentY][_currentX];
            if (!boardCell.revealed)
                boardCell.flagged = !boardCell.flagged;
            moved = true;
        }
        if (key == Key::R) {
            init();
            return;
        }
        if (moved) {
            _keyTimer = 0.f;
            break;
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
    const int remaining = MINES - flagCount();

    display.drawText("MINESWEEPER",
                     MinesweeperLayout::TITLE_X, MinesweeperLayout::TITLE_Y,
                     MinesweeperLayout::TITLE_COLOR, nullptr,
                     MinesweeperLayout::TITLE_SCALE);
    const std::string stats =
        "Mines:" + std::to_string(remaining) +
        "  Time:" + std::to_string((int)_elapsed) + "s" +
        "  Score:" + std::to_string(_score);
    display.drawText(stats,
                     MinesweeperLayout::STATS_X, MinesweeperLayout::STATS_Y,
                     MinesweeperLayout::STATS_COLOR, nullptr,
                     MinesweeperLayout::STATS_SCALE);

    static const Color cellBackground = Colors::DarkGray;

    std::vector<Cell> cells(COLS * ROWS);
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
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
                    if (MINES_FLAG_SIZE > 0) {
                        cell.imageData = MINES_FLAG_DATA;
                        cell.imageSize = MINES_FLAG_SIZE;
                    }
                } else {
                    cell.c       = '.';
                    cell.fg      = Colors::White;
                    cell.bg      = cellBackground;
                    cell.bgImage = cellBackground;
                    if (MINES_UNKNOWN_SIZE > 0) {
                        cell.imageData = MINES_UNKNOWN_DATA;
                        cell.imageSize = MINES_UNKNOWN_SIZE;
                    }
                }
            } else if (boardCell.mine) {
                cell.c       = '*';
                cell.fg      = Colors::Yellow;
                cell.bg      = Colors::DarkRed;
                cell.bgImage = Colors::DarkRed;
                if (MINES_BOMB_SIZE > 0) {
                    cell.imageData = MINES_BOMB_DATA;
                    cell.imageSize = MINES_BOMB_SIZE;
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

            cells[y * COLS + x] = cell;
        }
    }

    display.drawGrid(cells, COLS, ROWS,
                     MinesweeperConfig::ZONE_X1, MinesweeperConfig::ZONE_Y1,
                     MinesweeperConfig::ZONE_X2, MinesweeperConfig::ZONE_Y2,
                     _currentX, _currentY,
                     MinesweeperConfig::GRID_STYLE);

    display.drawText("ENTER:Reveal  SPC:Flag  R:Reset",
                     MinesweeperLayout::CONTROLS_X, MinesweeperLayout::CONTROLS_Y,
                     MinesweeperLayout::CONTROLS_COLOR, nullptr,
                     MinesweeperLayout::CONTROLS_SCALE);
}

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame()
    {
        return new Arcade::Minesweeper();
    }
    void destroyGame(Arcade::IGameModule* game)
    {
        delete game;
    }
}
