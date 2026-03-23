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
    bool operator==(const Point& other) const noexcept { return x == other.x && y == other.y; }
};

enum class Direction { UP, DOWN, LEFT, RIGHT };
enum class FoodType  { APPLE, ORANGE };

struct FoodItem {
    Point    pos;
    FoodType type;
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
    static constexpr int GRID_WIDTH  = SnakeConfig::GRID_WIDTH;
    static constexpr int GRID_HEIGHT = SnakeConfig::GRID_HEIGHT;

    bool _gameOver = false;
    int  _score    = 0;

    std::deque<Point>  _snake;
    Direction          _direction     = Direction::RIGHT;
    Direction          _nextDirection = Direction::RIGHT;
    std::vector<FoodItem> _foods;

    float _elapsed = 0.f;
    float _stepTimer  = 0.f;
    float _boostTimer = 0.f; ///< remaining seconds of orange speed boost

    bool _movedThisStep   = false;
    bool _ateFoodThisStep = false;
    FoodType _ateFoodType;

    void step();
    void spawnFood(FoodType type);
    void renderSnake(std::vector<Cell>& cells);

    bool isOnSnake(const Point& point) const noexcept;
    bool isOnFood (const Point& point) const noexcept;
};

} // namespace Arcade
