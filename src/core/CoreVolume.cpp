#include "Core.hpp"
#include <algorithm>
#include <string>

namespace Arcade {
    extern const unsigned char MENU_MUSIC_DATA[];
    extern const unsigned int  MENU_MUSIC_SIZE;
}

namespace Arcade {

void Core::playCurrentMusic()
{
    if (!_display || _musicBuffer.empty())
        return;
    _display->playMusic(_musicBuffer.data(), (unsigned int)_musicBuffer.size());
}

void Core::startMenuMusic()
{
    if (_display)
        _display->stopMusic(); // stop SFML streaming before _musicBuffer realloc
    if (MENU_MUSIC_SIZE > 0)
        _musicBuffer.assign(MENU_MUSIC_DATA, MENU_MUSIC_DATA + MENU_MUSIC_SIZE);
    else
        _musicBuffer.clear();
    playCurrentMusic();
}


void Core::handleVolumeKeys(const std::vector<Key>& keys)
{
    for (const auto key : keys) {
        if (key == Key::PLUS) {
            _volume = std::min(100, _volume + CoreConfig::VOLUME_STEP);
            _display->setVolume(_volume);
        } else if (key == Key::MINUS) {
            _volume = std::max(0, _volume - CoreConfig::VOLUME_STEP);
            _display->setVolume(_volume);
        }
    }
}

void Core::renderVolumeHint()
{
    if (!_display)
        return;

    const std::string text = "Vol: " + std::to_string(_volume) + "%";
    _display->drawText(text,
        CoreConfig::VOLUME_TEXT_X, CoreConfig::VOLUME_TEXT_Y,
        CoreConfig::VOLUME_TEXT_COLOR, nullptr,
        CoreConfig::VOLUME_TEXT_SCALE);
}

} // namespace Arcade
