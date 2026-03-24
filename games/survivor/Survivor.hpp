#pragma once
#include "IGameModule.hpp"
#include <cmath>
#include <deque>
#include <utility>
#include <vector>
#include <string>

namespace Arcade {

struct Point {
    int x = 0;
    int y = 0;
};

struct Star {
    Point pos;
};

enum class ProjShape { SQUARE, CIRCLE, TRIANGLE, LINE_H, LINE_V };

struct Projectile {
    Point     pos;
    int       size  = 2;
    int       vx    = 0;
    int       vy    = 1;
    ProjShape shape = ProjShape::SQUARE;
};

struct Explosion {
    Point pos;
    float timer = 0.f;
};

class Survivor final : public IGameModule {
public:
    void init()  override;
    void close() override {}
    void update(float deltaTime, const std::vector<Key>& keys) override;
    void render(IDisplayModule& display) override;
    void onMouseEvent(const MouseEvent& event) override;

    [[nodiscard]] bool        isGameOver() const override { return _gameOver; }
    [[nodiscard]] bool        hasWon()     const override { return false; }
    [[nodiscard]] int         getScore()   const override { return _score; }
    [[nodiscard]] std::string getName()    const override { return "Survivor"; }

    [[nodiscard]] const unsigned char* getMusicData() const override;
    [[nodiscard]] unsigned int         getMusicSize() const override;

private:
    void  spawnProjectile();
    bool  checkCollision(const Projectile& p) const;

    // Returns the set of (dx, dy) offsets occupied by a shape
    static std::vector<std::pair<int,int>> getShapeCells(ProjShape shape, int size);

    // Difficulty helpers
    [[nodiscard]] float difficulty()             const;
    [[nodiscard]] float currentSpawnPeriod()     const;
    [[nodiscard]] float currentEnemyMovePeriod() const;
    [[nodiscard]] int   currentProjVy()          const;

    bool _gameOver = false;
    int  _score    = 0;
    int  _health   = 5;
    int  _enemyRandomMove = 0;
    int  _enemyDirection  = 0;

    Point _player;
    Point _enemy;
    float _playerFX = 0.f;  ///< float X for delta-time movement
    float _playerFY = 0.f;  ///< float Y for delta-time movement

    std::vector<Star>       _stars;
    std::vector<Projectile> _projectiles;
    std::vector<Explosion>  _explosions;
    std::deque<Point>       _trail;

    bool _playExplosionSound = false;

    // Mouse tracking
    float _mouseTargetFX     = -1.f; ///< grid X target from mouse (-1 = not set)
    float _mouseTargetFY     = -1.f; ///< grid Y target from mouse (-1 = not set)
    float _mouseLastMoveTime = -999.f; ///< _elapsed value when mouse last moved

    float _elapsed        = 0.f;
    float _lastTrailTime  = 0.f;
    float _lastStarTime   = 0.f;
    float _lastSpawnTime  = 0.f;
    float _lastProjMove   = 0.f;
    float _lastEnemyMove  = 0.f;
};

} // namespace Arcade
