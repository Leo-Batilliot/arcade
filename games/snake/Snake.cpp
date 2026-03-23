#include "Snake.hpp"
#include "IDisplayModule.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace Arcade {
    extern const unsigned char SNAKE_MUSIC_DATA[];
    extern const unsigned int  SNAKE_MUSIC_SIZE;
    extern const unsigned char SNAKE_HEAD_DATA[];
    extern const unsigned int  SNAKE_HEAD_SIZE;
}

namespace Arcade {

void Snake::init()
{
    _gameOver      = false;
    _score         = 0;
    _direction     = Direction::RIGHT;
    _nextDirection = Direction::RIGHT;
    _stepTimer     = 0.f;
    _stepPeriod    = SnakeConfig::INITIAL_STEP_PERIOD;

    _snake.clear();
    const int centerX = GRID_WIDTH  / 2;
    const int centerY = GRID_HEIGHT / 2;
    for (int i = 0; i < SnakeConfig::INITIAL_LENGTH; ++i)
        _snake.push_back({centerX - i, centerY});

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    spawnFood();
}

const unsigned char* Snake::getMusicData() const
{
    return SNAKE_MUSIC_DATA;
}

unsigned int Snake::getMusicSize() const
{
    return SNAKE_MUSIC_SIZE;
}

void Snake::spawnFood()
{
    Point candidate;
    int attempts = 0;
    do {
        candidate.x = 1 + std::rand() % (GRID_WIDTH  - 2);
        candidate.y = 1 + std::rand() % (GRID_HEIGHT - 2);
    } while (isOnSnake(candidate) && ++attempts < GRID_WIDTH * GRID_HEIGHT);
    _food = candidate;
}

bool Snake::isOnSnake(const Point& point) const noexcept
{
    for (const auto& segment : _snake) {
        if (segment == point)
            return true;
    }
    return false;
}

void Snake::update(float deltaTime, const std::vector<Key>& keys)
{
    if (_gameOver)
        return;

    for (const auto key : keys) {
        if (key == Key::UP    && _direction != Direction::DOWN)
            _nextDirection = Direction::UP;
        if (key == Key::DOWN  && _direction != Direction::UP)
            _nextDirection = Direction::DOWN;
        if (key == Key::LEFT  && _direction != Direction::RIGHT)
            _nextDirection = Direction::LEFT;
        if (key == Key::RIGHT && _direction != Direction::LEFT)
            _nextDirection = Direction::RIGHT;
    }

    _stepTimer += deltaTime;
    if (_stepTimer < _stepPeriod)
        return;
    _stepTimer -= _stepPeriod;
    step();
}

void Snake::step()
{
    _direction = _nextDirection;
    Point head = _snake.front();

    switch (_direction) {
        case Direction::UP:    head.y--; break;
        case Direction::DOWN:  head.y++; break;
        case Direction::LEFT:  head.x--; break;
        case Direction::RIGHT: head.x++; break;
    }

    if (head.x < 0 || head.x >= GRID_WIDTH ||
        head.y < 0 || head.y >= GRID_HEIGHT) {
        _gameOver = true;
        return;
    }

    for (int i = 0; i < (int)_snake.size() - 1; ++i) {
        if (_snake[i] == head) {
            _gameOver = true;
            return;
        }
    }

    _snake.push_front(head);
    if (head == _food) {
        _score += SnakeConfig::SCORE_PER_FOOD;
        if (_score % SnakeConfig::SPEEDUP_SCORE_PERIOD == 0)
            _stepPeriod = std::max(SnakeConfig::MIN_STEP_PERIOD,
                                   _stepPeriod - SnakeConfig::STEP_PERIOD_DECREMENT);
        spawnFood();
    } else {
        _snake.pop_back();
    }
}

void Snake::render(IDisplayModule& display)
{
    std::vector<Cell> cells(GRID_WIDTH * GRID_HEIGHT);

    display.drawText("SNAKE",
                     SnakeLayout::TITLE_X, SnakeLayout::TITLE_Y,
                     SnakeLayout::TITLE_COLOR, nullptr,
                     SnakeLayout::TITLE_SCALE);

    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            const Color background = ((x + y) % 2 == 0)
                                   ? SnakeConfig::BG_DARK
                                   : SnakeConfig::BG_LIGHT;
            Cell& cell    = cells[y * GRID_WIDTH + x];
            cell.c        = ' ';
            cell.fg       = background;
            cell.bg       = background;
            cell.bgImage  = background;
        }
    }

    // Food
    cells[_food.y * GRID_WIDTH + _food.x].bg      = SnakeConfig::FOOD_COLOR;
    cells[_food.y * GRID_WIDTH + _food.x].bgImage = SnakeConfig::FOOD_COLOR;
    cells[_food.y * GRID_WIDTH + _food.x].fg      = SnakeConfig::FOOD_COLOR;

    // Snake body (all segments except head)
    for (std::size_t i = 1; i < _snake.size(); ++i) {
        const auto& segment = _snake[i];
        const Color natural = ((segment.x + segment.y) % 2 == 0)
                            ? SnakeConfig::BG_DARK
                            : SnakeConfig::BG_LIGHT;
        cells[segment.y * GRID_WIDTH + segment.x] =
            {' ', SnakeConfig::BODY_COLOR, SnakeConfig::BODY_COLOR, natural};
    }

    // Snake head with image and direction-based rotation
    if (!_snake.empty()) {
        const auto& head    = _snake.front();
        const Color natural = ((head.x + head.y) % 2 == 0)
                            ? SnakeConfig::BG_DARK
                            : SnakeConfig::BG_LIGHT;
        Cell& headCell     = cells[head.y * GRID_WIDTH + head.x];
        headCell.bg        = SnakeConfig::BODY_COLOR;
        headCell.bgImage   = natural;
        headCell.fg        = Colors::White;
        headCell.c         = '@';
        if (SNAKE_HEAD_SIZE > 0) {
            headCell.imageData = SNAKE_HEAD_DATA;
            headCell.imageSize = SNAKE_HEAD_SIZE;
            // Image faces LEFT by default; rotate/flip per direction
            switch (_direction) {
                case Direction::LEFT:
                    headCell.imageAngle = 0.f;
                    headCell.imageFlipX = false;
                    break;
                case Direction::RIGHT:
                    headCell.imageAngle = 0.f;
                    headCell.imageFlipX = true;
                    break;
                case Direction::UP:
                    headCell.imageAngle = 90.f;
                    headCell.imageFlipX = false;
                    break;
                case Direction::DOWN:
                    headCell.imageAngle = 270.f;
                    headCell.imageFlipX = false;
                    break;
            }
        }
    }

    const int focusX = _snake.empty() ? -1 : _snake.front().x;
    const int focusY = _snake.empty() ? -1 : _snake.front().y;
    display.drawGrid(cells, GRID_WIDTH, GRID_HEIGHT,
                     SnakeConfig::ZONE_X1, SnakeConfig::ZONE_Y1,
                     SnakeConfig::ZONE_X2, SnakeConfig::ZONE_Y2,
                     focusX, focusY,
                     SnakeConfig::GRID_STYLE);
}

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame()
    {
        return new Arcade::Snake();
    }
    void destroyGame(Arcade::IGameModule* game)
    {
        delete game;
    }
}
