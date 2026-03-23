#pragma once
#include "IDisplayModule.hpp"
#include "DisplayConfig.hpp"
#if __has_include(<SDL2/SDL.h>)
#  include <SDL2/SDL.h>
#elif __has_include(<SDL.h>)
#  include <SDL.h>
#else
#  error "SDL2 header not found"
#endif
#if __has_include(<SDL2/SDL_ttf.h>)
#  include <SDL2/SDL_ttf.h>
#elif __has_include(<SDL_ttf.h>)
#  include <SDL_ttf.h>
#else
#  error "SDL2_ttf header not found"
#endif
#if __has_include(<SDL2/SDL_mixer.h>)
#  include <SDL2/SDL_mixer.h>
#  define ARCADE_SDL2_MIXER 1
#elif __has_include(<SDL_mixer.h>)
#  include <SDL_mixer.h>
#  define ARCADE_SDL2_MIXER 1
#endif
#include <map>
#include <unordered_map>
#include <vector>

namespace Arcade {

/// @brief IDisplayModule implementation backed by SDL2 + SDL2_ttf.
class SDL2Display final : public IDisplayModule {
public:
    /// @brief Destroys the display, releasing all SDL resources.
    ~SDL2Display() override;

    /// @brief Initialises SDL2, creates the window and renderer.
    /// @param title Window title string.
    void init(const std::string& title) override;

    /// @brief Destroys the SDL2 window and renderer.
    void close() override;

    /// @brief Clears the renderer to black in preparation for a new frame.
    void clear() override;

    /// @brief Presents the current frame to the screen.
    void display() override;

    /// @brief Returns true while the SDL2 window is open.
    [[nodiscard]] bool isOpen() const override { return _open; }

    std::vector<Key>        getKeys()                    override;
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
    void drawRect(float x1, float y1, float x2, float y2, const Color& color) override;
#ifdef ARCADE_SDL2_IMAGES
    [[nodiscard]] bool supportsImages() const override { return true; }
    void clearImageCache() override;
#endif

    std::string getName() const override { return "SDL2"; }

private:
    static constexpr int WIN_W = DisplayConfig::WIN_W;
    static constexpr int WIN_H = DisplayConfig::WIN_H;

    SDL_Window*   _window   = nullptr;
    SDL_Renderer* _renderer = nullptr;
    bool          _open     = false;

    /// Lazy font cache: pixel-size -> TTF_Font* (loaded on demand from embedded data).
    std::map<int, TTF_Font*> _fontCache;

    float _lastRelX = -1.f;
    float _lastRelY = -1.f;
    float _prevRelX = -1.f;
    float _prevRelY = -1.f;
    std::vector<MouseEvent> _pendingMouseEvents;
    std::string _textInput;

    struct GridState {
        int   gridWidth  = 0;
        int   gridHeight = 0;
        float cellSize   = 0.f;
        int   originX    = 0;
        int   originY    = 0;
        int   viewWidth  = 0;
        int   viewHeight = 0;
        int   cameraX    = 0;
        int   cameraY    = 0;
    } _lastGrid;

#ifdef ARCADE_SDL2_MIXER
    Mix_Music* _music     = nullptr;
    Mix_Chunk* _chunk     = nullptr;
    bool       _mixerOpen = false;
#endif

#ifdef ARCADE_SDL2_IMAGES
    std::unordered_map<const unsigned char*, SDL_Texture*> _imageCache;
    SDL_Texture* getCachedImage(const unsigned char* data, unsigned int size);
#endif

    /// @brief Returns a cached TTF_Font for the given pixel size, loading lazily from embedded data.
    TTF_Font* getCachedFont(int size);
    Key       sdlKey(SDL_Keycode key) const noexcept;
    void      fillMouseEvent(MouseEvent& mouseEvent, int pixelX, int pixelY) const;
};

} // namespace Arcade
