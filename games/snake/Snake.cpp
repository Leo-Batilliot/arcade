#include "Snake.hpp"
#include "IDisplayModule.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace Arcade {
    extern const unsigned char SNAKEMUSIC_OGG_DATA[];
    extern const unsigned int  SNAKEMUSIC_OGG_SIZE;
    extern const unsigned char MOVE_OGG_DATA[];
    extern const unsigned int  MOVE_OGG_SIZE;
    extern const unsigned char EAT_OGG_DATA[];
    extern const unsigned int  EAT_OGG_SIZE;
    extern const unsigned char SPEED_UP_OGG_DATA[];
    extern const unsigned int  SPEED_UP_OGG_SIZE;
    extern const unsigned char SNAKE_HEAD_PNG_DATA[];
    extern const unsigned int  SNAKE_HEAD_PNG_SIZE;
    extern const unsigned char SNAKE_STRAIGHT_BODY_PNG_DATA[];
    extern const unsigned int  SNAKE_STRAIGHT_BODY_PNG_SIZE;
    extern const unsigned char SNAKE_CURVED_BODY_PNG_DATA[];
    extern const unsigned int  SNAKE_CURVED_BODY_PNG_SIZE;
    extern const unsigned char SNAKE_TAIL_PNG_DATA[];
    extern const unsigned int  SNAKE_TAIL_PNG_SIZE;
    extern const unsigned char APPLE_PNG_DATA[];
    extern const unsigned int  APPLE_PNG_SIZE;
    extern const unsigned char ORANGE_PNG_DATA[];
    extern const unsigned int  ORANGE_PNG_SIZE;
}

namespace Arcade {

// ── Image helpers ─────────────────────────────────────────────────────────────

static void applyImage(Cell& cell,
                       const unsigned char* data, unsigned int size,
                       float angle, bool flipX = false) noexcept
{
    if (size == 0)
        return;
    cell.imageData  = data;
    cell.imageSize  = size;
    cell.imageAngle = angle;
    cell.imageFlipX = flipX;
}

// ── Snake lifecycle ───────────────────────────────────────────────────────────

void Snake::init()
{
    _gameOver      = false;
    _score         = 0;
    _direction     = Direction::RIGHT;
    _nextDirection = Direction::RIGHT;
    _stepTimer     = 0.f;
    _boostTimer    = 0.f;

    _snake.clear();
    _foods.clear();
    const int centerX = GRID_WIDTH  / 2;
    const int centerY = GRID_HEIGHT / 2;
    for (int i = 0; i < SnakeConfig::INITIAL_LENGTH; ++i)
        _snake.push_back({centerX - i, centerY});

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < SnakeConfig::MAX_APPLES;  ++i) spawnFood(FoodType::APPLE);
    for (int i = 0; i < SnakeConfig::MAX_ORANGES; ++i) spawnFood(FoodType::ORANGE);
}

const unsigned char* Snake::getMusicData() const { return SNAKEMUSIC_OGG_DATA; }
unsigned int         Snake::getMusicSize()  const { return SNAKEMUSIC_OGG_SIZE; }

void Snake::spawnFood(FoodType type)
{
    Point candidate;
    int attempts = 0;
    do {
        candidate.x = 1 + std::rand() % (GRID_WIDTH  - 2);
        candidate.y = 1 + std::rand() % (GRID_HEIGHT - 2);
    } while ((isOnSnake(candidate) || isOnFood(candidate)) && ++attempts < GRID_WIDTH * GRID_HEIGHT);
    _foods.push_back({candidate, type});
}

bool Snake::isOnSnake(const Point& point) const noexcept
{
    for (const auto& segment : _snake)
        if (segment == point)
            return true;
    return false;
}

bool Snake::isOnFood(const Point& point) const noexcept
{
    for (const auto& food : _foods)
        if (food.pos == point)
            return true;
    return false;
}

void Snake::update(float deltaTime, const std::vector<Key>& keys)
{
    if (_gameOver)
        return;

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
            state.lastActionTime = _elapsed;
            executeAction = true;
        } else if (_elapsed - state.lastActionTime >= SnakeConfig::KEY_REPEAT_DELAY) {
            state.lastActionTime = _elapsed;
            executeAction = true;
        }

        if (!executeAction)
            continue;
        if (key == Key::UP    && _direction != Direction::DOWN)
            { _nextDirection = Direction::UP;    _movedThisStep = true; }
        if (key == Key::DOWN  && _direction != Direction::UP)
            { _nextDirection = Direction::DOWN;  _movedThisStep = true; }
        if (key == Key::LEFT  && _direction != Direction::RIGHT)
            { _nextDirection = Direction::LEFT;  _movedThisStep = true; }
        if (key == Key::RIGHT && _direction != Direction::LEFT)
            { _nextDirection = Direction::RIGHT; _movedThisStep = true; }
    }

    _boostTimer = std::max(0.f, _boostTimer - deltaTime);
    const float activePeriod = (_boostTimer > 0.f)
        ? SnakeConfig::STEP_PERIOD / SnakeConfig::ORANGE_SPEED_MULTIPLIER
        : SnakeConfig::STEP_PERIOD;

    _stepTimer += deltaTime;
    if (_stepTimer < activePeriod)
        return;
    _stepTimer -= activePeriod;
    step();
}

void Snake::step()
{
    _ateFoodThisStep = false;
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

    for (int i = 0; i < (int)_foods.size(); ++i) {
        if (_foods[i].pos == head) {
            const FoodType type = _foods[i].type;
            _foods.erase(_foods.begin() + i);
            if (type == FoodType::ORANGE) {
                _score      += SnakeConfig::SCORE_PER_FOOD * SnakeConfig::ORANGE_SCORE_MULTIPLIER;
                _boostTimer  = SnakeConfig::ORANGE_BOOST_DURATION;
            } else {
                _score += SnakeConfig::SCORE_PER_FOOD;
            }
            spawnFood(type);
            _ateFoodType = type;
            _ateFoodThisStep = true;
            break;
        }
    }
    if (!_ateFoodThisStep)
        _snake.pop_back();
}

void Snake::renderSnake(std::vector<Cell>& cells)
{
    const int snakeLen = (int)_snake.size();

    // ── Tail ──
    Point curr = _snake[snakeLen - 1];
    Point prev = _snake[snakeLen - 2];
    float angle = 0.0f;

    Cell& tailCell = cells[curr.y * GRID_WIDTH + curr.x];
    tailCell.bg = SnakeConfig::BODY_COLOR;
    if (prev.x < curr.x) angle = 90.0f;
    else if (prev.x > curr.x) angle = 270.0f;
    else if (prev.y < curr.y) angle = 180.0f;
    else angle = 0.0f;
    applyImage(tailCell, SNAKE_TAIL_PNG_DATA, SNAKE_TAIL_PNG_SIZE, angle);

    // ── Head ──
    curr = _snake[0];
    Point next = _snake[1];

    Cell& headCell = cells[curr.y * GRID_WIDTH + curr.x];
    headCell.bg = SnakeConfig::BODY_COLOR;
    headCell.c = 'o';
    if (next.x < curr.x) angle = 90.0f;
    else if (next.x > curr.x) angle = 270.0f;
    else if (next.y > curr.y) angle = 0.0f;
    else angle = 180.0f;
    applyImage(headCell, SNAKE_HEAD_PNG_DATA, SNAKE_HEAD_PNG_SIZE, angle);

    // ── Body ──
    prev = _snake[0];
    for (int i = 1; i < snakeLen - 1; ++i) {
        curr = _snake[i];
        next = _snake[i + 1];
        Cell& bodyCell = cells[curr.y * GRID_WIDTH + curr.x];
        bodyCell.bg = SnakeConfig::BODY_COLOR;

        if (prev.x == next.x || prev.y == next.y) {
            if (prev.x == next.x) angle = 90.0f;
            else angle = 0.0f;
            applyImage(bodyCell, SNAKE_STRAIGHT_BODY_PNG_DATA, SNAKE_STRAIGHT_BODY_PNG_SIZE, angle);
        } else {
            if ((prev.x == curr.x - 1 && next.y == curr.y + 1) || (next.x == curr.x - 1 && prev.y == curr.y + 1)) angle = 180.0f;
            else if ((prev.x == curr.x - 1 && next.y == curr.y - 1) || (next.x == curr.x - 1 && prev.y == curr.y - 1)) angle = 270.0f;
            else if ((prev.x == curr.x + 1 && next.y == curr.y + 1) || (next.x == curr.x + 1 && prev.y == curr.y + 1)) angle = 90.0f;
            else angle = 0.0f;
            applyImage(bodyCell, SNAKE_CURVED_BODY_PNG_DATA, SNAKE_CURVED_BODY_PNG_SIZE, angle);
        }
        prev = curr;
    }
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void Snake::render(IDisplayModule& display)
{
    if (_ateFoodThisStep) {
        display.playSound(EAT_OGG_DATA, EAT_OGG_SIZE);
        _ateFoodThisStep = false;
        _movedThisStep   = false;
        if (_ateFoodType == FoodType::ORANGE)
            display.playSound(SPEED_UP_OGG_DATA, SPEED_UP_OGG_SIZE);
    } else if (_movedThisStep) {
        display.playSound(MOVE_OGG_DATA, MOVE_OGG_SIZE);
        _movedThisStep = false;
    }

    std::vector<Cell> cells(GRID_WIDTH * GRID_HEIGHT);

    // Background checkerboard
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            const Color bg = ((x + y) % 2 == 0) ? SnakeConfig::BG_DARK : SnakeConfig::BG_LIGHT;
            Cell& cell   = cells[y * GRID_WIDTH + x];
            cell.c       = ' ';
            cell.fg      = bg;
            cell.bg      = bg;
            cell.bgImage = bg;
        }
    }

    // Food items
    for (const auto& food : _foods) {
        Cell& cell = cells[food.pos.y * GRID_WIDTH + food.pos.x];
        if (food.type == FoodType::APPLE) {
            cell.bg = SnakeConfig::FOOD_COLOR;
            applyImage(cell, APPLE_PNG_DATA, APPLE_PNG_SIZE, 0.f);
        } else {
            cell.bg = SnakeConfig::ORANGE_COLOR;
            applyImage(cell, ORANGE_PNG_DATA, ORANGE_PNG_SIZE, 0.f);
        }
    }

    renderSnake(cells);

    const int focusX = _snake.empty() ? -1 : _snake.front().x;
    const int focusY = _snake.empty() ? -1 : _snake.front().y;

    display.drawGrid(cells, GRID_WIDTH, GRID_HEIGHT,
                     SnakeConfig::ZONE_X1, SnakeConfig::ZONE_Y1,
                     SnakeConfig::ZONE_X2, SnakeConfig::ZONE_Y2,
                     focusX, focusY,
                     SnakeConfig::GRID_STYLE);
    display.drawText("SNAKE",
                     SnakeLayout::TITLE_X, SnakeLayout::TITLE_Y,
                     SnakeLayout::TITLE_COLOR, nullptr,
                     SnakeLayout::TITLE_SCALE);
    
    const std::string stats =
        "Snake size:" + std::to_string(_snake.size()) +
        "       Time:" + std::to_string((int)_elapsed) + "s" +
        "       Score:" + std::to_string(_score);
    display.drawText(stats,
                     SnakeLayout::STATS_X, SnakeLayout::STATS_Y,
                     SnakeLayout::STATS_COLOR, nullptr,
                     SnakeLayout::STATS_SCALE);

    display.drawText("Arrows: Move",
                     SnakeLayout::CONTROLS_X, SnakeLayout::CONTROLS_Y,
                     SnakeLayout::CONTROLS_COLOR, nullptr,
                     SnakeLayout::CONTROLS_SCALE);
}

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame()  { return new Arcade::Snake(); }
    void destroyGame(Arcade::IGameModule* game) { delete game; }
}
