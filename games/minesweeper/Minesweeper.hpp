#pragma once
#include "IGameModule.hpp"
#include "GameConfig.hpp"
#include <vector>

namespace Arcade {

/// @brief Classic Minesweeper game module.
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
    bool        hasWon()     const override { return _won; }
    int         getScore()   const override { return _score; }
    std::string getName()    const override { return "Minesweeper"; }

private:
    static constexpr int   COLS      = MinesweeperConfig::COLS;
    static constexpr int   ROWS      = MinesweeperConfig::ROWS;
    static constexpr int   MINES     = MinesweeperConfig::MINES;
    static constexpr float KEY_DELAY = MinesweeperConfig::KEY_DELAY;

    struct BoardCell {
        bool mine     = false;
        bool revealed = false;
        bool flagged  = false;
        int  adjacent = 0;
    };

    bool  _gameOver  = false;
    bool  _won       = false;
    bool  _started   = false;
    int   _score     = 0;
    float _elapsed   = 0.f;
    float _keyTimer  = 0.f;

    int _currentX = COLS / 2;
    int _currentY = ROWS / 2;

    std::vector<std::vector<BoardCell>> _board;

    /// @brief Distributes mines randomly; safeX/safeY and their neighbours are guaranteed mine-free.
    void  placeMines(int safeX, int safeY);
    void  computeAdjacent();
    /// @brief Recursively reveals blank neighbours (flood-fill) when adjacent count is 0.
    void  reveal(int x, int y);
    void  checkWin();
    int   flagCount() const noexcept;
    static Color adjacentColor(unsigned char count) noexcept;
};

} // namespace Arcade
