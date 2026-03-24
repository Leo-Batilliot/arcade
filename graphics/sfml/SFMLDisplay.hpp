#pragma once
#include "IDisplayModule.hpp"
#include "DisplayConfig.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Arcade {

/// @brief IDisplayModule implementation backed by SFML.
class SFMLDisplay final : public IDisplayModule {
public:
    /// @brief Destroys the display, closing the window if still open.
    ~SFMLDisplay() override;

    /// @brief Creates and opens the SFML render window.
    /// @param title Window title string.
    void init(const std::string& title) override;

    /// @brief Closes the SFML render window.
    void close() override;

    /// @brief Clears the window to black in preparation for a new frame.
    void clear() override;

    /// @brief Presents the current frame to the screen.
    void display() override;

    /// @brief Returns true while the SFML window is open.
    [[nodiscard]] bool isOpen() const override;

    std::vector<Key>        getKeys()                    override;
    std::vector<Key>        getHeldKeys()                override;
    std::vector<MouseEvent> getMouseEvents()             override;
    std::pair<float, float> getMousePosition()     const override;
    bool                    hasMouseMoved()        const override;

    void drawGrid(const std::vector<Cell>& cells, int gridWidth, int gridHeight,
                  float x1, float y1, float x2, float y2,
                  int focusX = -1, int focusY = -1,
                  const GridStyle& style = {}) override;
    void drawText(const std::string& text, float relativeX, float relativeY,
                  const Color& fg, const Color* bg = nullptr,
                  float scale = 1.0f) override;
    void drawLine(char c, float x1, float x2, float y,
                  const Color& fg, const Color& bg,
                  float scale = 1.0f) override;

    void playMusic(const unsigned char* data, unsigned int size, bool loop = true) override;
    void playSound(const unsigned char* data, unsigned int size) override;
    void stopMusic() override;
    void setVolume(int percent) override;

    [[nodiscard]] std::string getTextInput() override;
    [[nodiscard]] bool supportsImages() const override { return true; }
    void clearImageCache() override;
    void drawRect(float x1, float y1, float x2, float y2, const Color& color) override;

    std::string getName() const override { return "SFML"; }

private:
    static constexpr int WIN_W = DisplayConfig::WIN_W;
    static constexpr int WIN_H = DisplayConfig::WIN_H;

    std::unique_ptr<sf::RenderWindow> _window;
    sf::Font _font;
    bool     _fontLoaded = false;

    float _lastRelX = -1.f;
    float _lastRelY = -1.f;
    float _prevRelX = -1.f;
    float _prevRelY = -1.f;
    std::vector<MouseEvent> _pendingMouseEvents;
    std::string _textInput;
    std::unordered_map<const unsigned char*, sf::Texture> _imageCache;
    const sf::Texture* getCachedImage(const unsigned char* data, unsigned int size);

    struct GridState {
        int   gridWidth = 0;
        int   gridHeight = 0;
        float cellSize = 0.f;
        int   originX = 0;
        int   originY = 0;
        int   viewWidth = 0;
        int   viewHeight = 0;
        int   cameraX = 0;
        int   cameraY = 0;
    } _lastGrid;

    bool                         _audioEnabled = false; ///< false when no playback device found
    std::optional<sf::Music>     _music;               ///< recreated each load to avoid corrupted state
    bool                         _musicOpen    = false;
    sf::SoundBuffer              _soundBuffer;
    std::optional<sf::Sound>     _sound;

    sf::Color toSFML(const Color& c) const noexcept;
    Key       sfmlKey(sf::Keyboard::Key key) const noexcept;
    void      fillMouseEvent(MouseEvent& mouseEvent, int pixelX, int pixelY) const;
};

} // namespace Arcade
