#pragma once
#include "IDisplayModule.hpp"
#include "DisplayConfig.hpp"
#include <ncurses.h>
#include <vector>
#include <cstdint>
#include <chrono>
#include <map>

namespace Arcade {

/// @brief IDisplayModule implementation backed by ncurses.
class NCursesDisplay final : public IDisplayModule {
public:
    NCursesDisplay() = default;

    /// @brief Destroys the display, calling close() if still open.
    ~NCursesDisplay() override;

    /// @brief Initialises ncurses and configures the terminal.
    /// @param title Ignored by the ncurses backend (terminals have no title bar).
    void init(const std::string& title) override;

    /// @brief Restores the terminal to its original state and ends ncurses.
    void close() override;

    /// @brief Clears the ncurses virtual screen.
    void clear() override;

    /// @brief Refreshes the terminal output with the current virtual screen.
    void display() override;

    /// @brief Returns true while the ncurses session is active.
    [[nodiscard]] bool isOpen() const override { return _isOpen; }

    /// @brief Mouse events are collected inside getKeys() to mirror SFML/SDL2 behaviour.
    std::vector<Key>        getKeys()                    override;
    std::vector<Key>        getHeldKeys()                override;
    std::vector<MouseEvent> getMouseEvents()             override;
    std::pair<float, float> getMousePosition()     const override;
    bool                    hasMouseMoved()        const override;

    void drawGrid(const std::vector<Cell>& cells, int gridWidth, int gridHeight,
                  float x1, float y1, float x2, float y2,
                  int focusX = -1, int focusY = -1,
                  const GridStyle& style = {}) override;
    /// @brief scale maps to ncurses attributes: >=2.0 bold+standout, >=1.5 bold, <=0.7 dim.
    void drawText(const std::string& text, float relX, float relY,
                  const Color& fg, const Color* bg = nullptr,
                  float scale = 1.0f) override;
    void drawLine(char c, float x1, float x2, float y,
                  const Color& fg, const Color& bg,
                  float scale = 1.0f) override;

    void drawRect(float x1, float y1, float x2, float y2, const Color& color) override;

    std::string getName() const override { return "ncurses"; }
    [[nodiscard]] std::string getTextInput() override;

    /// @brief Reclaims color pair slots to prevent exhaustion of COLOR_PAIRS.
    ///
    /// Call between scenes or levels when the palette is expected to change
    /// significantly.
    void resetColorPairs();

private:
    bool _isOpen       = false;
    bool _has256Colors = false;
    int  _terminalWidth  = NCursesConfig::DEFAULT_TERMINAL_WIDTH;
    int  _terminalHeight = NCursesConfig::DEFAULT_TERMINAL_HEIGHT;

    float _lastRelX = -1.f;
    float _lastRelY = -1.f;
    float _prevRelX = -1.f;
    float _prevRelY = -1.f;

    std::vector<MouseEvent> _pendingMouseEvents;
    std::string _textInput;

    using Clock = std::chrono::steady_clock;
    std::map<Key, Clock::time_point> _keyTimestamps; ///< last time each key was seen in getKeys()

    struct GridState {
        int gridWidth  = 0;
        int gridHeight = 0;
        int charWidth  = 1;
        int charHeight = 1;
        int originCol  = 0;
        int originRow  = 0;
        int viewWidth  = 0;
        int viewHeight = 0;
        int cameraX    = 0;
        int cameraY    = 0;
    } _lastGrid;

    /// Stable color-pair registry; entries are never evicted mid-frame.
    /// Key: (fg256index, bg256index)  Value: ncurses pair number.
    std::map<std::pair<int, int>, int> _colorPairs;
    int _nextPairNum = 8; ///< Pairs 1-7 are reserved for 8-color fallback.


    /// @brief Allocates an ncurses color pair on first use; falls back gracefully when COLOR_PAIRS is full.
    int    getOrCreatePair(int fgIdx, int bgIdx);
    chtype cellAttr(const Color& fg, const Color& bg) noexcept;
    chtype scaleAttr(float scale) const noexcept;
    void   drawBlock(int row, int col, int numRows, int numCols,
                     char c, chtype attr, int clipRow0, int clipRow1);
    void   fillMouseEvent(MouseEvent& mouseEvent, int termX, int termY) const;
};

} // namespace Arcade
