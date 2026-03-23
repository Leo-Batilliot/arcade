# Arcade

A modular arcade platform in C++17 that dynamically loads graphical and game libraries at runtime.

## Collaborating group

> **Interface collaboration e-mail:** `leo.batilliot@epitech.eu.`

## Dependencies

### Auto-downloaded by CMake (no install needed)

The following libraries are fetched and compiled automatically via CMake FetchContent
if they are not already installed on the system:

| Library | Used by | Notes |
|---------|---------|-------|
| SFML 3 | `arcade_sfml.so` | Graphics + audio (miniaudio bundled) |
| SDL2 | `arcade_sdl2.so` | Graphics |
| SDL2_ttf | `arcade_sdl2.so` | Font rendering |
| SDL2_mixer | `arcade_sdl2.so` | Audio mixing |
| NCurses 6.4 | `arcade_ncurses.so` | Terminal rendering |

### Required build tools (must be installed manually)

```bash
sudo apt install build-essential cmake git
```

### Required for SDL2 audio (cannot be handled by CMake)

SDL2 audio drivers (`pulse`, `alsa`) are compiled **into** SDL2 at build time and require
system development headers to be present **before** SDL2 is compiled. CMake cannot install
these automatically.

Without them, SDL2 will have no working audio driver (only the silent `dummy` driver).

```bash
# Install at least one of:
sudo apt install libpulse-dev       # PulseAudio (recommended)
sudo apt install libasound2-dev     # ALSA (alternative)
```

After installing, **delete the build directory and rebuild from scratch**:

```bash
rm -rf build && mkdir build && cd build && cmake .. && make -j$(nproc)
```

> **Note:** SFML audio works out of the box without these packages — SFML 3 bundles
> its own audio engine (miniaudio) internally.

---

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..
```

After a successful build:

```
arcade                    ← main executable
lib/
  arcade_ncurses.so
  arcade_sfml.so
  arcade_sdl2.so
  arcade_snake.so
  arcade_minesweeper.so
```

### Partial build targets

```bash
cmake --build . --target core        # executable only
cmake --build . --target graphicals  # display libraries only
cmake --build . --target games       # game libraries only
```

---

## Usage

```bash
./arcade ./lib/arcade_sfml.so
./arcade ./lib/arcade_ncurses.so
./arcade ./lib/arcade_sdl2.so
```

---

## Controls

### Global (all states)

| Key | Action |
|-----|--------|
| `TAB` | Cycle to next graphical library |
| `ESC` | Return to menu / quit from menu |
| `+` / `=` | Volume +10% |
| `-` | Volume -10% |

### Menu

| Key | Action |
|-----|--------|
| `↑` / `↓` | Navigate game list |
| `Alphabetic keys` | Change player name |
| `Enter` | Launch selected game |

### In-game

| Key | Action |
|-----|--------|
| Arrow keys | Move / control |
| `Space` | Game-specific action |
| `ESC` | Return to menu |
| `TAB` | Cycle to next graphical library |
| `g` | Cycle to next game |
| `r` | Restart current game |

---

## Project structure

```
arcade/
├── include/
│   ├── Types.hpp           — shared types (Key, Color, Cell, MouseEvent)
│   ├── IDisplayModule.hpp  — display backend interface
│   └── IGameModule.hpp     — game module interface
├── src/
│   ├── main.cpp
│   ├── Core.hpp / core/    — main loop, library management
│   ├── CoreConfig.hpp      — HUD/menu layout constants
│   ├── DLLoader.hpp        — dlopen wrapper
│   └── assets/             — embedded assets (menu music)
├── graphics/
│   ├── DisplayConfig.hpp   — shared display constants
│   ├── assets/             — shared graphical assets (fonts)
│   ├── ncurses/            — NCurses backend
│   ├── sfml/               — SFML backend
│   └── sdl2/               — SDL2 backend
├── games/
│   ├── GameConfig.hpp      — game layout constants
│   ├── snake/              — Snake game + assets
│   └── minesweeper/        — Minesweeper game + assets
├── lib/                    — compiled shared libraries
└── CMakeLists.txt
```
