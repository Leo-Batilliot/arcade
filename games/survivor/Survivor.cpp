#include "Survivor.hpp"
#include "IDisplayModule.hpp"
#include "GameConfig.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace Arcade {
    extern const unsigned char SURVIVORMUSIC_OGG_DATA[];
    extern const unsigned int  SURVIVORMUSIC_OGG_SIZE;
    extern const unsigned char EXPLOSION_OGG_DATA[];
    extern const unsigned int  EXPLOSION_OGG_SIZE;
}

namespace Arcade {

const unsigned char* Survivor::getMusicData() const { return SURVIVORMUSIC_OGG_DATA; }
unsigned int         Survivor::getMusicSize() const  { return SURVIVORMUSIC_OGG_SIZE; }

// ─── shape cells ──────────────────────────────────────────────────────────────
std::vector<std::pair<int,int>> Survivor::getShapeCells(ProjShape shape, int size)
{
    std::vector<std::pair<int,int>> cells;
    const float cx = (size - 1) / 2.f;
    const float cy = (size - 1) / 2.f;
    const float r  = size / 2.f;

    switch (shape) {
        case ProjShape::SQUARE:
            for (int dy = 0; dy < size; ++dy)
                for (int dx = 0; dx < size; ++dx)
                    cells.push_back({dx, dy});
            break;

        case ProjShape::CIRCLE:
            for (int dy = 0; dy < size; ++dy)
                for (int dx = 0; dx < size; ++dx)
                    if ((dx - cx)*(dx - cx) + (dy - cy)*(dy - cy) <= r * r + 0.25f)
                        cells.push_back({dx, dy});
            break;

        case ProjShape::TRIANGLE:
            // Apex at top-center, widens toward bottom
            for (int dy = 0; dy < size; ++dy) {
                const int half   = (size > 1) ? dy * (size / 2) / (size - 1) : 0;
                const int center = (size - 1) / 2;
                for (int dx = center - half; dx <= center + half; ++dx)
                    if (dx >= 0 && dx < size)
                        cells.push_back({dx, dy});
            }
            break;

        case ProjShape::LINE_H:
            for (int dx = 0; dx < size; ++dx)
                cells.push_back({dx, size / 2});
            break;

        case ProjShape::LINE_V:
            for (int dy = 0; dy < size; ++dy)
                cells.push_back({size / 2, dy});
            break;
    }
    return cells;
}

// ─── difficulty helpers ───────────────────────────────────────────────────────
float Survivor::difficulty() const
{
    const float level = std::floor(_elapsed / SurvivorConfig::DIFFICULTY_LEVEL_DURATION);
    return std::min(level, SurvivorConfig::MAX_DIFFICULTY);
}

float Survivor::currentSpawnPeriod() const
{
    const float period = SurvivorConfig::PROJ_SPAWN_PERIOD
        / (1.f + difficulty() * SurvivorConfig::PROJ_SPAWN_SPEED_MULT);
    return std::max(period, SurvivorConfig::MIN_SPAWN_PERIOD);
}

float Survivor::currentEnemyMovePeriod() const
{
    const float period = SurvivorConfig::ENEMY_MOVE_TIMER
        / (1.f + difficulty() * SurvivorConfig::ENEMY_SPEED_MULT);
    return std::max(period, SurvivorConfig::MIN_ENEMY_MOVE_PERIOD);
}

int Survivor::currentProjVy() const
{
    const int vy = 1 + static_cast<int>(difficulty() * SurvivorConfig::PROJ_SPEED_MULT);
    return std::min(vy, SurvivorConfig::MAX_PROJ_VY);
}

// ─── onMouseEvent ─────────────────────────────────────────────────────────────
void Survivor::onMouseEvent(const MouseEvent& event)
{
    // Only track hover/movement (no button press needed)
    if (event.button != Key::NONE) return;

    // Map screen-relative coords to game grid coords
    constexpr float zoneW = SurvivorConfig::ZONE_X2 - SurvivorConfig::ZONE_X1;
    constexpr float zoneH = SurvivorConfig::ZONE_Y2 - SurvivorConfig::ZONE_Y1;

    const float gridRelX = (event.relativeX - SurvivorConfig::ZONE_X1) / zoneW;
    const float gridRelY = (event.relativeY - SurvivorConfig::ZONE_Y1) / zoneH;

    _mouseTargetFX = std::clamp(gridRelX * SurvivorConfig::GRID_W,
                                0.f, static_cast<float>(SurvivorConfig::GRID_W - 1));
    _mouseTargetFY = std::clamp(gridRelY * SurvivorConfig::GRID_H,
                                0.f, static_cast<float>(SurvivorConfig::GRID_H - 1));
    _mouseLastMoveTime = _elapsed;
}

// ─── init ─────────────────────────────────────────────────────────────────────
void Survivor::init()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    _gameOver           = false;
    _score              = 0;
    _health             = SurvivorConfig::INITIAL_HEALTH;
    _elapsed            = 0.f;
    _lastTrailTime      = 0.f;
    _lastStarTime       = 0.f;
    _lastSpawnTime      = 0.f;
    _lastProjMove       = 0.f;
    _lastEnemyMove      = 0.f;
    _playExplosionSound  = false;
    _enemyRandomMove     = 0;
    _enemyDirection      = 0;
    _mouseTargetFX       = -1.f;
    _mouseTargetFY       = -1.f;
    _mouseLastMoveTime   = -999.f;

    _stars.clear();
    _projectiles.clear();
    _explosions.clear();
    _trail.clear();

    _playerFX = static_cast<float>(SurvivorConfig::GRID_W / 2);
    _playerFY = static_cast<float>(SurvivorConfig::INITIAL_PLAYER_Y);
    _player   = { (int)_playerFX, (int)_playerFY };
    _enemy    = { SurvivorConfig::GRID_W / 2, SurvivorConfig::INITIAL_ENEMY_Y };

    for (int y = 0; y < SurvivorConfig::GRID_H; y++)
        for (int i = 0; i < 4 * SurvivorConfig::STARS_PER_LINE_AVERAGE; i++)
            if (std::rand() % 4 == 0)
                _stars.push_back({{ std::rand() % SurvivorConfig::GRID_W, y }});
}

// ─── update ───────────────────────────────────────────────────────────────────
void Survivor::update(float deltaTime, const std::vector<Key>& keys)
{
    if (_gameOver) return;

    _elapsed += deltaTime;

    // Tick down active explosions
    for (auto& e : _explosions)
        e.timer -= deltaTime;
    _explosions.erase(
        std::remove_if(_explosions.begin(), _explosions.end(),
            [](const Explosion& e){ return e.timer <= 0.f; }),
        _explosions.end());

    // ── Player movement (delta-time: mouse tracking + keyboard) ───────────────
    float dx = 0.f, dy = 0.f;

    // Mouse tracking: move toward cursor if it moved recently
    const bool mouseActive = _mouseTargetFX >= 0.f
        && (_elapsed - _mouseLastMoveTime) < SurvivorConfig::MOUSE_ACTIVE_DURATION;
    if (mouseActive) {
        const float mdx  = _mouseTargetFX - _playerFX;
        const float mdy  = _mouseTargetFY - _playerFY;
        const float dist = std::sqrt(mdx * mdx + mdy * mdy);
        if (dist > SurvivorConfig::MOUSE_DEAD_ZONE) {
            dx = mdx / dist;
            dy = mdy / dist;
        }
    }

    // Arrow keys add to (or replace) the mouse direction
    for (const auto key : keys) {
        if (key == Key::LEFT)  dx -= 1.f;
        if (key == Key::RIGHT) dx += 1.f;
        if (key == Key::UP)    dy -= 1.f;
        if (key == Key::DOWN)  dy += 1.f;
    }

    // Normalize final direction vector if magnitude > 1
    const float mag = std::sqrt(dx * dx + dy * dy);
    if (mag > 1.f) { dx /= mag; dy /= mag; }

    _playerFX += dx * SurvivorConfig::PLAYER_SPEED * deltaTime;
    _playerFY += dy * SurvivorConfig::PLAYER_SPEED * deltaTime;
    _playerFX = std::clamp(_playerFX,
        0.f,
        static_cast<float>(SurvivorConfig::GRID_W - SurvivorConfig::PLAYER_RIGHT_MARGIN));
    _playerFY = std::clamp(_playerFY,
        static_cast<float>(SurvivorConfig::PLAYER_UPPER_LIMIT),
        static_cast<float>(SurvivorConfig::GRID_H - SurvivorConfig::PLAYER_BOTTOM_MARGIN));
    _player.x = static_cast<int>(_playerFX);
    _player.y = static_cast<int>(_playerFY);

    // Update exhaust trail at fixed period
    if (_elapsed - _lastTrailTime >= SurvivorConfig::TRAIL_UPDATE_PERIOD) {
        _trail.push_front({
            _player.x + SurvivorConfig::EXHAUST_OFFSET_X,
            _player.y + SurvivorConfig::EXHAUST_OFFSET_Y
        });
        if ((int)_trail.size() > SurvivorConfig::TRAIL_LENGTH)
            _trail.pop_back();
        _lastTrailTime = _elapsed;
    }

    // ── Enemy tracks player (center-to-center, fixed ±1 step) ─────────────────
    if (_elapsed - _lastEnemyMove >= currentEnemyMovePeriod()) {
        const int enemyCenter  = _enemy.x + SurvivorConfig::ENEMY_SPRITE_HALF_W;
        const int playerCenter = _player.x + SurvivorConfig::PLAYER_SPRITE_HALF_W;

        if (_enemyRandomMove != 0) {
            _enemy.x += _enemyDirection;
            _enemyRandomMove--;
        }
        else if (enemyCenter < playerCenter - SurvivorConfig::ENEMY_RANDOMNESS_DISTANCE) {
            _enemy.x++;
        }
        else if (enemyCenter > playerCenter + SurvivorConfig::ENEMY_RANDOMNESS_DISTANCE) {
            _enemy.x--;
        }
        else {
            _enemyRandomMove = SurvivorConfig::ENEMY_RANDOM_MOVE_COUNT;
            _enemyDirection  = (std::rand() % 2 == 0) ? 1 : -1;
        }
        _enemy.x = std::clamp(_enemy.x, 0, SurvivorConfig::GRID_W - 1);
        _lastEnemyMove = _elapsed;
    }

    // ── Projectile spawning ───────────────────────────────────────────────────
    if (_elapsed - _lastSpawnTime >= currentSpawnPeriod()) {
        spawnProjectile();
        _lastSpawnTime = _elapsed;
    }

    // ── Projectile movement + collision ───────────────────────────────────────
    _playExplosionSound = false;
    if (_elapsed - _lastProjMove >= SurvivorConfig::PROJ_MOVE_PERIOD) {
        for (auto it = _projectiles.begin(); it != _projectiles.end();) {
            it->pos.x += it->vx;
            it->pos.y += it->vy;

            if (checkCollision(*it)) {
                _health--;
                _explosions.push_back({
                    { it->pos.x + it->size / 2, it->pos.y + it->size / 2 },
                    SurvivorConfig::EXPLOSION_DURATION
                });
                _playExplosionSound = true;
                it = _projectiles.erase(it);
                continue;
            }
            if (it->pos.y >= SurvivorConfig::GRID_H ||
                it->pos.x < 0 || it->pos.x >= SurvivorConfig::GRID_W) {
                it = _projectiles.erase(it);
            } else {
                ++it;
            }
        }
        _lastProjMove = _elapsed;
    }

    // ── Star field ────────────────────────────────────────────────────────────
    if (_elapsed - _lastStarTime >= SurvivorConfig::STAR_MOVE_PERIOD) {
        for (auto& s : _stars) s.pos.y++;
        _stars.erase(
            std::remove_if(_stars.begin(), _stars.end(),
                [](const Star& s){ return s.pos.y >= SurvivorConfig::GRID_H; }),
            _stars.end());
        for (int i = 0; i < 4 * SurvivorConfig::STARS_PER_LINE_AVERAGE; i++)
            if (std::rand() % 4 == 0)
                _stars.push_back({{ std::rand() % SurvivorConfig::GRID_W, 0 }});
        _lastStarTime = _elapsed;
    }

    if (_health <= 0)
        _gameOver = true;
    _score = static_cast<int>(_elapsed * 10.f * (difficulty() + 1));
}

// ─── spawnProjectile ──────────────────────────────────────────────────────────
void Survivor::spawnProjectile()
{
    static const ProjShape SHAPES[] = {
        ProjShape::SQUARE, ProjShape::CIRCLE,
        ProjShape::TRIANGLE, ProjShape::LINE_H, ProjShape::LINE_V
    };

    Projectile projectile;
    projectile.pos   = { _enemy.x + SurvivorConfig::ENEMY_PROJ_OFFSET_X,
                _enemy.y + SurvivorConfig::ENEMY_PROJ_OFFSET_Y };
    projectile.size  = SurvivorConfig::PROJ_MIN_SIZE + (std::rand() % SurvivorConfig::PROJ_SIZE_RANGE);
    projectile.vy    = currentProjVy();
    projectile.vx    = (std::rand() % SurvivorConfig::PROJ_VX_RANGE) - SurvivorConfig::PROJ_VX_RANGE / 2;
    projectile.shape = SHAPES[std::rand() % 5];
    _projectiles.push_back(projectile);
}

// ─── checkCollision ───────────────────────────────────────────────────────────
bool Survivor::checkCollision(const Projectile& projectile) const
{
    const auto& sprite = SurvivorConfig::PLAYER_ASCII;
    for (const auto& [dx, dy] : getShapeCells(projectile.shape, projectile.size)) {
        const int relX = projectile.pos.x + dx - _player.x;
        const int relY = projectile.pos.y + dy - _player.y;
        if (relY >= 0 && relY < (int)sprite.size() &&
            relX >= 0 && relX < (int)sprite[relY].size()) {
            if (sprite[relY][relX] != ' ')
                return true;
        }
    }
    return false;
}

// ─── render ───────────────────────────────────────────────────────────────────
void Survivor::render(IDisplayModule& display)
{
    constexpr int GW = SurvivorConfig::GRID_W;
    constexpr int GH = SurvivorConfig::GRID_H;

    std::vector<Cell> cells(GW * GH);

    auto inBounds = [](int x, int y){ return x >= 0 && x < GW && y >= 0 && y < GH; };
    auto setCell  = [&](int x, int y, char c, const Color& fg) {
        if (inBounds(x, y)) { cells[y * GW + x].c = c; cells[y * GW + x].fg = fg; }
    };

    for (unsigned int i = 0; i < cells.size(); i++)
        cells[i].bg = SurvivorConfig::BACKGROUND_COLOR;
    // ── Stars ─────────────────────────────────────────────────────────────────
    for (const auto& s : _stars)
        setCell(s.pos.x, s.pos.y, '.', SurvivorConfig::STAR_COLOR);

    // ── Trail (oldest → newest so newest wins on overlap) ─────────────────────
    const int trail_len = static_cast<int>(_trail.size());
    for (int i = trail_len - 1; i >= 0; --i) {
        const auto& pos = _trail[i];
        const float t = 1.f - static_cast<float>(i) / SurvivorConfig::TRAIL_LENGTH;
        const auto& base = SurvivorConfig::TRAIL_COLOR;
        const Color col = {
            static_cast<uint8_t>(base.r * t),
            static_cast<uint8_t>(base.g * t),
            static_cast<uint8_t>(base.b * t),
            255
        };
        const char ch = (i < 2) ? '|' : (i < 4) ? ':' : '.';
        setCell(pos.x,     pos.y, ch, col);
        setCell(pos.x + 1, pos.y, ch, col);
    }

    // ── Projectiles (shape-aware) ─────────────────────────────────────────────
    for (const auto& projectile : _projectiles) {
        const Color& col = [&]() -> const Color& {
            switch (projectile.shape) {
                case ProjShape::CIRCLE:   return SurvivorConfig::PROJ_CIRCLE_COLOR;
                case ProjShape::TRIANGLE: return SurvivorConfig::PROJ_TRIANGLE_COLOR;
                case ProjShape::LINE_H:
                case ProjShape::LINE_V:   return SurvivorConfig::PROJ_LINE_COLOR;
                default:                  return SurvivorConfig::PROJ_COLOR;
            }
        }();
        const char ch = [&]() {
            switch (projectile.shape) {
                case ProjShape::CIRCLE:   return 'O';
                case ProjShape::TRIANGLE: return 'v';
                case ProjShape::LINE_H:   return '_';
                case ProjShape::LINE_V:   return '|';
                default:                  return '#';
            }
        }();
        for (const auto& [dx, dy] : getShapeCells(projectile.shape, projectile.size))
            setCell(projectile.pos.x + dx, projectile.pos.y + dy, ch, col);
    }

    // ── ASCII sprites ─────────────────────────────────────────────────────────
    auto drawAscii = [&](int ox, int oy,
                         const std::vector<std::string>& art, const Color& col) {
        for (int y = 0; y < (int)art.size(); ++y)
            for (int x = 0; x < (int)art[y].size(); ++x)
                if (art[y][x] != ' ')
                    setCell(ox + x, oy + y, art[y][x], col);
    };

    drawAscii(_player.x, _player.y, SurvivorConfig::PLAYER_ASCII, SurvivorConfig::PLAYER_COLOR);
    drawAscii(_enemy.x,  _enemy.y,  SurvivorConfig::ENEMY_ASCII,  SurvivorConfig::ENEMY_COLOR);

    // ── Explosions (drawn last — on top of everything) ────────────────────────
    const auto& expArt = SurvivorConfig::EXPLOSION_ASCII;
    for (const auto& e : _explosions) {
        for (int y = 0; y < (int)expArt.size(); ++y) {
            for (int x = 0; x < (int)expArt[y].size(); ++x) {
                const char c = expArt[y][x];
                const Color& col = (c == 'o')
                    ? SurvivorConfig::EXPLOSION_CORE_COLOR
                    : SurvivorConfig::EXPLOSION_COLOR;
                setCell(e.pos.x + x - 1, e.pos.y + y - 1, c, col);
            }
        }
    }

    // ── Draw grid ─────────────────────────────────────────────────────────────
    display.drawGrid(cells, GW, GH,
                     SurvivorConfig::ZONE_X1, SurvivorConfig::ZONE_Y1,
                     SurvivorConfig::ZONE_X2, SurvivorConfig::ZONE_Y2,
                     _player.x, _player.y, SurvivorConfig::GRID_STYLE);

    // ── Sound effects ─────────────────────────────────────────────────────────
    if (_playExplosionSound) {
        display.playSound(EXPLOSION_OGG_DATA, EXPLOSION_OGG_SIZE);
        _playExplosionSound = false;
    }

    // ── HUD ───────────────────────────────────────────────────────────────────

    // Title
    display.drawText("SURVIVOR",
                     SurvivorLayout::TITLE_X, SurvivorLayout::TITLE_Y,
                     SurvivorLayout::TITLE_COLOR, nullptr,
                     SurvivorLayout::TITLE_SCALE);

    // Stats line: score, elapsed time, difficulty level
    const int diffLevel = static_cast<int>(difficulty());
    const std::string stats =
        "Score: " + std::to_string(_score) +
        "   Time: " + std::to_string(static_cast<int>(_elapsed)) + "s" +
        "   Difficulty: " + std::to_string(diffLevel);
    display.drawText(stats,
                     SurvivorLayout::STATS_X, SurvivorLayout::STATS_Y,
                     SurvivorLayout::STATS_COLOR, nullptr,
                     SurvivorLayout::STATS_SCALE);

    // HP label
    display.drawText("HP:",
                     SurvivorLayout::HP_LABEL_X, SurvivorLayout::HP_BAR_Y,
                     SurvivorLayout::HP_FG_COLOR, nullptr,
                     SurvivorLayout::HP_SCALE);

    // HP bar: filled portion
    const float barWidth = SurvivorLayout::HP_BAR_X2 - SurvivorLayout::HP_BAR_X1;
    const float fillRatio = std::clamp(
        static_cast<float>(_health) / SurvivorConfig::INITIAL_HEALTH, 0.f, 1.f);
    const float fillX2 = SurvivorLayout::HP_BAR_X1 + barWidth * fillRatio;

    if (fillRatio > 0.f)
        display.drawLine(' ',
                         SurvivorLayout::HP_BAR_X1, fillX2,
                         SurvivorLayout::HP_BAR_Y,
                         SurvivorLayout::HP_FG_COLOR, SurvivorLayout::HP_BG_FULL,
                         SurvivorLayout::HP_SCALE);
    if (fillRatio < 1.f)
        display.drawLine(' ',
                         fillX2, SurvivorLayout::HP_BAR_X2,
                         SurvivorLayout::HP_BAR_Y,
                         SurvivorLayout::HP_FG_COLOR, SurvivorLayout::HP_BG_EMPTY,
                         SurvivorLayout::HP_SCALE);

    // Controls
    display.drawText("Arrows: Move",
                     SurvivorLayout::CONTROLS_X, SurvivorLayout::CONTROLS_Y,
                     SurvivorLayout::CONTROLS_COLOR, nullptr,
                     SurvivorLayout::CONTROLS_SCALE);
}

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame()               { return new Arcade::Survivor(); }
    void destroyGame(Arcade::IGameModule* game)     { delete game; }
}
