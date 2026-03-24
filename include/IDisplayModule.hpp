#pragma once
#include "Types.hpp"
#include <string>
#include <vector>

namespace Arcade {

/**
 * @brief Interface for graphical display backends.
 *
 * All positioning uses screen-ratio coordinates [0..1] so that games and
 * Core are completely independent of the backend's actual resolution or
 * terminal size — the backend is solely responsible for the mapping.
 *
 * Typical frame loop:
 *   display.clear()
 *   game.render(display)   // calls drawGrid / drawText / drawLine
 *   display.display()
 */
class IDisplayModule {
public:
    virtual ~IDisplayModule() = default;

    /// Open the display with the given window title.
    virtual void init(const std::string& title) = 0;

    /// Release all display resources.
    virtual void close() = 0;

    /// Clear the screen to black (start of frame).
    virtual void clear() = 0;

    /// Present the rendered frame.
    virtual void display() = 0;

    /// Returns true while the window / terminal is open.
    [[nodiscard]] virtual bool isOpen() const = 0;

    // ── Input ─────────────────────────────────────────────────────────────────

    /// Returns all keyboard events queued since last call.
    [[nodiscard]] virtual std::vector<Key> getKeys() = 0;

    /// Returns all keys currently held down (hardware-polled state).
    /// Unlike getKeys() which returns edge-triggered events, this returns every
    /// key that is physically pressed right now.
    /// SFML/SDL2: real hardware state (no delay, fully combinable).
    /// ncurses: timeout-based approximation (key stays "held" ~400 ms after last event).
    [[nodiscard]] virtual std::vector<Key> getHeldKeys() { return {}; }

    /// Returns mouse click/release events.  Each event carries both:
    ///   - relativeX/relativeY : screen-ratio position [0..1]
    ///   - gridX/gridY         : cell coordinates in the last drawGrid() call (−1 if outside)
    [[nodiscard]] virtual std::vector<MouseEvent> getMouseEvents() { return {}; }

    /// Current mouse cursor position as screen ratios.  Returns {−1,−1} if unknown.
    [[nodiscard]] virtual std::pair<float,float> getMousePosition() const { return {-1.f,-1.f}; }

    /// True if the mouse moved since last getKeys() call.
    [[nodiscard]] virtual bool hasMouseMoved() const { return false; }

    // ── Rendering ─────────────────────────────────────────────────────────────

    /// @brief Draw an entire game grid.
    ///
    /// All visual parameters (zone, background, highlight) are provided by the
    /// caller — the display backend makes no aesthetic decisions.
    virtual void drawGrid(
        const std::vector<Cell>& cells,
        int gridWidth, int gridHeight,
        float x1, float y1, float x2, float y2,
        int focusX = -1, int focusY = -1,
        const GridStyle& style = {}) = 0;

    /// @brief Draw text centred on a screen-ratio position; scale multiplies the backend base font size.
    virtual void drawText(
        const std::string& text,
        float relativeX, float relativeY,
        const Color& fg,
        const Color* bg = nullptr,
        float scale = 1.0f) = 0;

    /// @brief Draw a horizontal line; scale controls thickness (pixel) or bold/dim (ncurses).
    virtual void drawLine(
        char c,
        float x1, float x2, float y,
        const Color& fg, const Color& bg,
        float scale = 1.0f) = 0;

    // ── Audio ──────────────────────────────────────────────────────────────────

    /// @brief Start playing background music from an in-memory audio buffer.
    virtual void playMusic(
        [[maybe_unused]] const unsigned char* data,
        [[maybe_unused]] unsigned int size,
        [[maybe_unused]] bool loop = true) { }

    /// @brief Play a one-shot sound effect from an in-memory audio buffer.
    virtual void playSound(
        [[maybe_unused]] const unsigned char* data,
        [[maybe_unused]] unsigned int size) { }

    /// @brief Stop the currently playing background music immediately.
    virtual void stopMusic() {}

    /// @brief Set the master music volume.
    /// @param percent  Volume level in the range [0, 100].
    virtual void setVolume([[maybe_unused]] int percent) {}

    /// @brief Returns text typed by the user since the last call (respects shift/caps-lock).
    [[nodiscard]] virtual std::string getTextInput() { return ""; }

    /// @brief Returns true if this backend can render Cell::imageData as a sprite.
    [[nodiscard]] virtual bool supportsImages() const { return false; }

    /// @brief Frees all cached image textures. Call when a game is unloaded.
    virtual void clearImageCache() {}

    /// @brief Fill a screen-ratio rectangle with a color (supports alpha for overlays).
    virtual void drawRect(float x1, float y1, float x2, float y2,
                          const Color& color) { (void)x1;(void)y1;(void)x2;(void)y2;(void)color; }

    /// Backend name (e.g. "SDL2", "ncurses").
    [[nodiscard]] virtual std::string getName() const = 0;
};

} // namespace Arcade

extern "C" {
    Arcade::IDisplayModule* createDisplay();
    void destroyDisplay(Arcade::IDisplayModule* display);
}
