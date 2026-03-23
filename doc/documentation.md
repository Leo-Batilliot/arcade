# Arcade — Developer Documentation

## 1. Architecture overview

The Arcade platform is split into three independent layers that communicate exclusively through the two pure-virtual interfaces stored in `include/`:

```
┌─────────────────────────────────────────────────────┐
│                     arcade  (core)                  │
│  Core  ──► IDisplayModule*  (loaded from ./lib/)    │
│        ──► IGameModule*     (loaded from ./lib/)    │
└──────────────────┬──────────────────────────────────┘
                   │ dlopen / dlsym / dlclose
      ┌────────────┴──────────────┐
      │  ./lib/                   │
      │  arcade_ncurses.so        │
      │  arcade_sfml.so           │   ← implement IDisplayModule
      │  arcade_sdl2.so           │
      │                           │
      │  arcade_snake.so          │   ← implement IGameModule
      │  arcade_minesweeper.so    │
      └───────────────────────────┘
```

**Key design decisions**

- Games have **zero** knowledge of the display backend. They call `IDisplayModule::drawGrid()`, `drawText()`, `drawLine()` — all using screen-ratio coordinates [0..1].
- The core uses **RAII** (`DLLoader`) to manage shared library handles; `-fno-gnu-unique` ensures `dlclose` fully unloads each `.so`.
- All positions use **screen-ratio space [0..1]** so layouts are backend-independent (pixel window or terminal).
- Music is owned by Core (`_musicBuffer`) to avoid use-after-free when a game `.so` is unloaded.

---

## 2. Interface reference

### 2.1 `IDisplayModule`  (`include/IDisplayModule.hpp`)

| Method | Description |
|--------|-------------|
| `init(title)` | Open the window / initialise the terminal. |
| `close()` | Release all resources, close the window. |
| `clear()` | Clear the back-buffer to black. |
| `display()` | Flip / refresh — present the rendered frame. |
| `isOpen() → bool` | Returns `false` once the window is closed. |
| `getKeys() → vector<Key>` | Poll and return all keys pressed since last call. |
| `getMouseEvents() → vector<MouseEvent>` | Click/release events with screen-ratio + grid coordinates. |
| `getMousePosition() → pair<float,float>` | Current cursor position as ratios; `{-1,-1}` if unknown. |
| `hasMouseMoved() → bool` | True if cursor moved since last `getKeys()` call. |
| `drawGrid(cells, w, h, x1, y1, x2, y2)` | Render a game grid within the given bounding box (screen ratios). |
| `drawText(text, relX, relY, fg, bg*, scale)` | Draw text centred at a screen-ratio position. |
| `drawLine(c, x1, x2, y, fg, bg)` | Draw a horizontal line of character `c`. |
| `playMusic(data, size, loop)` | Start playing background music from memory. |
| `playSound(data, size)` | Play a one-shot sound effect. |
| `stopMusic()` | Stop background music immediately. |
| `setVolume(percent)` | Set music volume [0..100]. |
| `getName() → string` | Human-readable backend name (e.g. "SFML"). |

**C entry-points to export:**
```cpp
extern "C" {
    Arcade::IDisplayModule* createDisplay();
    void destroyDisplay(Arcade::IDisplayModule* display);
}
```

### 2.2 `IGameModule`  (`include/IGameModule.hpp`)

| Method | Description |
|--------|-------------|
| `init()` | Initialise / reset the game state. |
| `close()` | Release all game resources. |
| `update(dt, keys)` | Advance game logic by `dt` seconds with the given key set. |
| `render(display)` | Draw the current game state through `display`. |
| `onMouseEvent(event)` | Optional: handle mouse click / hover events. |
| `getMusicData() → const unsigned char*` | Return pointer to embedded audio data (may return `nullptr`). |
| `getMusicSize() → unsigned int` | Return byte length of the audio buffer. |
| `isGameOver() → bool` | `true` once the player has lost. |
| `hasWon() → bool` | `true` once the player has won. |
| `getScore() → int` | Current integer score. |
| `getName() → string` | Human-readable game name (used as score key). |

**C entry-points to export:**
```cpp
extern "C" {
    Arcade::IGameModule* createGame();
    void destroyGame(Arcade::IGameModule* game);
}
```

---

## 3. Data types  (`include/Types.hpp`)

### `Key` enum
Canonical key identifiers independent of any backend.

| Group | Values |
|-------|--------|
| Navigation | `UP DOWN LEFT RIGHT` |
| Control | `ENTER ESCAPE SPACE BACKSPACE TAB` |
| Letters | `A … Z` |
| Symbols | `PLUS MINUS` |
| Function | `F1 … F12` |
| Numpad | `NUM_0 … NUM_9` |
| Mouse | `MOUSE_LEFT MOUSE_MIDDLE MOUSE_RIGHT` |

### `MouseEvent` struct
```cpp
struct MouseEvent {
    float relX, relY;   // screen-ratio position [0..1]
    int   gridX, gridY; // cell coordinates in last drawGrid() (-1 = outside)
    Key   button;       // MOUSE_LEFT / MOUSE_RIGHT / NONE (hover)
};
```

### `Color` struct
```cpp
struct Color { uint8_t r, g, b, a; };
```
Pre-defined palette in `Arcade::Colors::` (`Black`, `White`, `Red`, `Green`, `Blue`, `Yellow`, `Cyan`, `Gray`, …).

### `Cell` struct
```cpp
struct Cell { char c; Color fg; Color bg; };
```
One cell in a game grid — character with foreground and background colours.

---

## 4. Key bindings (runtime)

| Key | State | Action |
|-----|-------|--------|
| `↑` / `↓` | Menu | Move game selection cursor |
| `ENTER` | Menu | Launch selected game |
| `TAB` | Menu / Game | Cycle to next graphical backend |
| `ESC` | Menu | Exit the program |
| `ESC` | Game | Return to menu |
| `R` | Game | Restart the current game |
| `G` | Game | Switch to the next game |
| `+` | Any | Increase volume by 10% |
| `-` | Any | Decrease volume by 10% |
| Letters | Menu | Edit player name |
| `BACKSPACE` | Menu | Delete last character of player name |
| Any key | Game Over | Return to menu |

---

## 5. Adding a new graphical library — step by step

1. **Create a directory** `graphics/<name>/`.
2. **Write a class** inheriting `Arcade::IDisplayModule` (all pure-virtual methods must be implemented).
3. **Export the C entry-points** (see §2.1).
4. **Add a `CMakeLists.txt`**:
   ```cmake
   add_library(arcade_<name> SHARED MyDisplay.cpp)
   target_include_directories(arcade_<name> PRIVATE
       ${CMAKE_SOURCE_DIR}/include
       ${CMAKE_SOURCE_DIR}/src
       ${CMAKE_CURRENT_SOURCE_DIR})
   target_link_libraries(arcade_<name> PRIVATE <your_lib>)
   target_compile_options(arcade_<name> PRIVATE
       -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)
   set_target_properties(arcade_<name> PROPERTIES PREFIX "" OUTPUT_NAME "arcade_<name>")
   ```
5. **Register in the root `CMakeLists.txt`**:
   ```cmake
   add_subdirectory(graphics/<name>)
   ```
   And add `arcade_<name>` to the `graphicals` custom target dependency list.
6. Build → `./lib/arcade_<name>.so` is detected and listed automatically at runtime.

**Coordinate system reminder:** All `drawText`, `drawLine`, and `drawGrid` positions use **screen-ratio [0..1]** (0 = left/top, 1 = right/bottom). Map these to your backend's actual pixel or cell dimensions inside each draw method.

**Audio:** Implement `playMusic`, `playSound`, `stopMusic`, and `setVolume` if your backend supports audio; otherwise the default no-op stubs in the interface are sufficient.

---

## 6. Adding a new game library — step by step

1. **Create a directory** `games/<name>/`.
2. **Write a class** inheriting `Arcade::IGameModule`.
3. **Implement every virtual method** (see §2.2).
4. **Export the C entry-points** (see §2.2).
5. **(Optional) Embed background music:**
   Place an OGG Vorbis file at `games/<name>/assets/<name>music.ogg`, then in `CMakeLists.txt`:
   ```cmake
   file(READ "assets/<name>music.ogg" _HEX HEX)
   # ... generate a .gen.cpp with the byte array ...
   ```
   Override `getMusicData()` and `getMusicSize()` to return the embedded buffer.
6. **Add a `CMakeLists.txt`**:
   ```cmake
   add_library(arcade_<name> SHARED MyGame.cpp)
   target_include_directories(arcade_<name> PRIVATE
       ${CMAKE_SOURCE_DIR}/include
       ${CMAKE_SOURCE_DIR}/games
       ${CMAKE_CURRENT_SOURCE_DIR})
   target_compile_options(arcade_<name> PRIVATE
       -Wall -Wextra -Wpedantic -Wno-unused-parameter -fno-gnu-unique)
   set_target_properties(arcade_<name> PROPERTIES PREFIX "" OUTPUT_NAME "arcade_<name>")
   ```
7. **Register in the root `CMakeLists.txt`**:
   ```cmake
   add_subdirectory(games/<name>)
   ```
   And add `arcade_<name>` to the `games` custom target dependency list.
8. Build → `./lib/arcade_<name>.so` appears in the menu automatically.

**Genericity note:** Games must call only `IDisplayModule` methods — no SDL2, SFML, or ncurses headers in game code.

---

## 7. Class diagram (textual)

```
«interface»                         «interface»
IDisplayModule                      IGameModule
──────────────────────────────      ────────────────────────
+ init(title)                       + init()
+ close()                           + close()
+ clear() / display()               + update(dt, keys)
+ isOpen() : bool                   + render(display&)
+ getKeys() : vector<Key>           + onMouseEvent(event)
+ getMouseEvents()                  + getMusicData() : const uchar*
+ getMousePosition()                + getMusicSize()  : uint
+ hasMouseMoved()                   + isGameOver() : bool
+ drawGrid(cells,w,h,x1,y1,x2,y2)  + hasWon()     : bool
+ drawText(text,x,y,fg,bg*,scale)   + getScore()   : int
+ drawLine(c,x1,x2,y,fg,bg)         + getName()    : string
+ playMusic / playSound
+ stopMusic / setVolume
+ getName() : string

      ▲                                   ▲
      │ implements                        │ implements
┌────────────────┐  ┌────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ NCursesDisplay │  │ SFMLDisplay│  │      Snake        │  │   Minesweeper    │
└────────────────┘  └────────────┘  └──────────────────┘  └──────────────────┘
                    ┌────────────┐
                    │ SDL2Display│
                    └────────────┘

Core
────────────────────────────────────────────────────────
- _displayLibs : vector<LibInfo>   (path + name)
- _gameLibs    : vector<LibInfo>
- _display     : IDisplayModule*   (owned via DLLoader + destroyer fn ptr)
- _game        : IGameModule*      (owned via DLLoader + destroyer fn ptr)
- _musicBuffer : vector<uchar>     (owned copy — survives dlclose)
- _scores      : map<string, ScoreEntry>
- _playerName  : string
- _volume      : int               (0..100)
- _state       : enum { MENU, PLAYING, GAMEOVER }
+ Core(initialDisplayLib)
+ run()
- scanLibraries()
- loadDisplay(idx) / unloadDisplay() / nextDisplay()
- loadGame(idx)    / unloadGame()   / nextGame()
- restartGame()
- runMenu()  / renderMenu()
- runGame()  / renderHUD()
- runGameOver() / renderGameOverScreen()
- loadScores() / saveScores() / updateScore(name, score)
- handleVolumeKeys() / renderVolumeHint()
- playCurrentMusic() / startMenuMusic()

DLLoader
────────
- _handle : void*   (dlopen result; RTLD_NOW | RTLD_LOCAL)
+ DLLoader(path)    (throws std::runtime_error on failure)
+ getSymbol<FuncPtr>(name) : FuncPtr
+ hasSymbol(name)          : bool
+ ~DLLoader()              (calls dlclose)
```

---

## 8. Build system summary

```bash
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build                    # everything
cmake --build build --target core        # arcade binary only
cmake --build build --target graphicals  # display .so files only
cmake --build build --target games       # game .so files only
```

Outputs after build:
```
./arcade               ← main executable
./lib/
  arcade_ncurses.so
  arcade_sfml.so
  arcade_sdl2.so
  arcade_snake.so
  arcade_minesweeper.so
  deps/                ← SDL2 / SFML runtime libs (only if auto-downloaded)
```

**Portability note:** Each `arcade_*.so` can be distributed independently. If SDL2 or SFML are system-installed, no `deps/` folder is required. If they were auto-downloaded, ship `lib/deps/` alongside the `.so`.
