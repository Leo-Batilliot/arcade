#pragma once
#include "Types.hpp"
#include <string>
#include <vector>

namespace Arcade {

class IDisplayModule; // forward declaration

/**
 * @brief Interface for game modules.
 *
 * Each game is loaded as a shared library and must export createGame() /
 * destroyGame().  The game owns its grid size and tells the display where
 * to draw via drawGrid(x1,y1,x2,y2) bounding-box ratios.
 */
class IGameModule {
public:
    virtual ~IGameModule() = default;

    /// Initialise (or re-initialise) the game.
    virtual void init() = 0;

    /// Release game resources.
    virtual void close() = 0;

    /// Advance game logic by @p deltaTime seconds.
    virtual void update(float deltaTime, const std::vector<Key>& keys) = 0;

    /// Render the current game state to the display.
    virtual void render(IDisplayModule& display) = 0;

    /// Optional mouse event handler (hover events have button == NONE).
    virtual void onMouseEvent([[maybe_unused]] const MouseEvent& event) {}

    /// @brief Returns a pointer to the game's embedded background music data.
    ///
    /// Core copies this data into its own buffer immediately after loadGame(),
    /// so the returned pointer only needs to be valid until init() returns.
    /// Return nullptr (default) if the game has no background music.
    [[nodiscard]] virtual const unsigned char* getMusicData() const { return nullptr; }

    /// @brief Returns the byte length of the music buffer returned by getMusicData().
    [[nodiscard]] virtual unsigned int getMusicSize() const { return 0; }

    [[nodiscard]] virtual bool        isGameOver() const = 0;
    [[nodiscard]] virtual bool        hasWon()     const = 0;
    [[nodiscard]] virtual int         getScore()   const = 0;
    [[nodiscard]] virtual std::string getName()    const = 0;
};

} // namespace Arcade

extern "C" {
    Arcade::IGameModule* createGame();
    void destroyGame(Arcade::IGameModule* game);
}
