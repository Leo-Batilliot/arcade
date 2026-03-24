#pragma once
#include "IGameModule.hpp"
#include "GameConfig.hpp"
#include <map>
#include <vector>
#include <unordered_map>

namespace Arcade {

/// @brief Classic Minesweeper game module with infinite escalating levels.
class Minesweeper final : public IGameModule {
public:
    void init()    override;
    void close()   override {}
    void update(float deltaTime, const std::vector<Key>& keys) override;
    void render(IDisplayModule& display)                       override;
    void onMouseEvent(const MouseEvent& event)                 override;

    [[nodiscard]] const unsigned char* getMusicData() const override;
    [[nodiscard]] unsigned int         getMusicSize() const override;

    bool        isGameOver() const override { return _gameOver; }
    bool        hasWon()     const override { return false; }
    int         getScore()   const override { return _score; }
    std::string getName()    const override { return "Minesweeper"; }

private:

    struct BoardCell {
        bool mine     = false;
        bool revealed = false;
        bool flagged  = false;
        int  adjacent = 0;
    };

    // Dynamic grid dimensions (grow each level)
    int _cols  = MinesweeperConfig::INITIAL_SIZE;
    int _rows  = MinesweeperConfig::INITIAL_SIZE;
    int _mines = 0;
    int _level = 1;

    bool  _gameOver = false;
    bool  _started  = false;
    int   _score    = 0;
    float _elapsed  = 0.f;
    float _totalTime = 0.f;
    
    struct KeyState {
        bool isPressed = false;
        float lastActionTime = 0.f;
    };

    std::unordered_map<Key, KeyState> _keyStates;

    int _currentX = 0;
    int _currentY = 0;

    std::vector<std::vector<BoardCell>> _board;

    // Sound triggers — set during update/event, consumed in render
    bool _soundFlag      = false;
    bool _soundSafe      = false;
    bool _soundBomb      = false;
    bool _soundNextLevel = false;

    /// @brief Computes mine count for given grid size.
    static int minesForSize(int cols, int rows) noexcept;

    /// @brief Computes the next odd grid side length after applying growth factor.
    static int nextOddSize(int current) noexcept;

    /// @brief Initialises the board for the current _cols/_rows/_mines without resetting score/level.
    void initBoard();

    void  placeMines(int safeX, int safeY);
    void  computeAdjacent();

    /// @brief Recursively reveals blank neighbours (flood-fill) when adjacent count is 0.
    /// @return true if a safe cell was newly revealed (for sound purposes).
    bool  reveal(int x, int y);

    void  checkWin();
    int   flagCount() const noexcept;
    static Color adjacentColor(unsigned char count) noexcept;
};

} // namespace Arcade
