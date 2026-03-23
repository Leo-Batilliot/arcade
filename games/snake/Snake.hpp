#pragma once
#include "IGameModule.hpp"
#include "GameConfig.hpp"
#include <deque>
#include <vector>

namespace Arcade {

/// @brief Integer 2-D grid coordinate.
struct Point {
    int x = 0;
    int y = 0;
    /// @brief Returns true when both coordinates are equal.
    bool operator==(const Point& other) const noexcept { return x == other.x && y == other.y; }
};

/// @brief Classic Snake game module.
class Snake final : public IGameModule {
public:
    void init()    override;
    void close()   override {}
    void update(float deltaTime, const std::vector<Key>& keys) override;
    void render(IDisplayModule& display)                       override;

    [[nodiscard]] const unsigned char* getMusicData() const override;
    [[nodiscard]] unsigned int         getMusicSize() const override;

    bool        isGameOver() const override { return _gameOver; }
    bool        hasWon()     const override { return false; }
    int         getScore()   const override { return _score; }
    std::string getName()    const override { return "Snake"; }

private:
    enum class Direction { UP, DOWN, LEFT, RIGHT };

    static constexpr int GRID_WIDTH  = SnakeConfig::GRID_WIDTH;
    static constexpr int GRID_HEIGHT = SnakeConfig::GRID_HEIGHT;

    bool _gameOver = false;
    int  _score    = 0;

    std::deque<Point> _snake;
    Direction         _direction     = Direction::RIGHT;
    Direction         _nextDirection = Direction::RIGHT;
    Point             _food          = {};

    float _stepTimer  = 0.f;
    float _stepPeriod = SnakeConfig::INITIAL_STEP_PERIOD;

    /// @brief Advances the snake by one cell in the current direction.
    void step();

    /// @brief Places the food token on a random cell not occupied by the snake.
    void spawnFood();

    /// @brief Returns true if the given point overlaps any segment of the snake.
    bool isOnSnake(const Point& point) const noexcept;
};

} // namespace Arcade
