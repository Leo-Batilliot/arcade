#include "NCursesDisplay.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace Arcade {

// ─── Color helpers ────────────────────────────────────────────────────────────

// Map an RGB color to the xterm-256 color cube index.
// Indices 16-231: 6×6×6 color cube.
// Indices 232-255: 24-step grayscale ramp.
static int rgbTo256(const Color& c)
{
    const int red   = c.r;
    const int green = c.g;
    const int blue  = c.b;

    if (red + green + blue < 30)
        return COLOR_BLACK;
    // Grayscale: all channels within ±10 of each other
    if (std::abs(red - green) <= 10 && std::abs(green - blue) <= 10)
    {
        const int gray = (red + green + blue) / 3;
        return 232 + (gray * 24) / 256;
    }
    // Color cube: each axis is 0-5
    const int redIndex   = (red   * 6) / 256;
    const int greenIndex = (green * 6) / 256;
    const int blueIndex  = (blue  * 6) / 256;
    return 16 + 36 * redIndex + 6 * greenIndex + blueIndex;
}

// Map an RGB color to one of the 8 basic terminal colors.
static int rgbToBasic(const Color& c)
{
    const int red   = c.r;
    const int green = c.g;
    const int blue  = c.b;

    if (red + green + blue < 30)
        return COLOR_BLACK;
    if (red + green + blue > 630)
        return COLOR_WHITE;
    if (red >= green && red >= blue)
        return (green >= 80) ? COLOR_YELLOW : COLOR_RED;
    if (green >= red && green >= blue)
        return (blue >= 80) ? COLOR_CYAN : COLOR_GREEN;
    if (blue >= red && blue >= green)
        return (red >= 80) ? COLOR_MAGENTA : COLOR_BLUE;
    return COLOR_WHITE;
}

// ─── 8-color fallback table ───────────────────────────────────────────────────
// Maps each basic COLOR_* constant to its reserved pair number.
//   pair 1 = WHITE on BLACK    pair 5 = BLUE on BLACK
//   pair 2 = RED on BLACK      pair 6 = MAGENTA on BLACK
//   pair 3 = GREEN on BLACK    pair 7 = CYAN on BLACK
//   pair 4 = YELLOW on BLACK
static int basicColorToPair(int basicColor)
{
    switch (basicColor)
    {
        case COLOR_RED:     return 2;
        case COLOR_GREEN:   return 3;
        case COLOR_YELLOW:  return 4;
        case COLOR_BLUE:    return 5;
        case COLOR_MAGENTA: return 6;
        case COLOR_CYAN:    return 7;
        case COLOR_WHITE:   return 1;
        case COLOR_BLACK:
        default:            return 1;
    }
}

NCursesDisplay::~NCursesDisplay()
{
    close();
}

// ─── init / close ─────────────────────────────────────────────────────────────
void NCursesDisplay::init(const std::string& /*title*/)
{
    // Always end any active ncurses session before reinitialising.
    // We test the ncurses global flag rather than _isOpen so that a
    // previous NCursesDisplay instance's endwin() is properly followed
    // by a fresh initscr() without any mode-restoration in between.
    // reset_prog_mode() / reset_shell_mode() must NOT be called here:
    // they would put the terminal into a stale curses raw-mode state
    // that causes initscr() to take a "resume" path and skip the full
    // color-table reset, producing corrupted colors on reload.
    if (!isendwin())
        endwin();

    set_escdelay(NCursesConfig::ESCAPE_DELAY_MS);

    initscr();

    if (has_colors())
    {
        start_color();
        use_default_colors();

        // Wipe ALL color-pair slots that may have survived from a previous
        // ncurses session.  initscr() reuses the SCREEN structure without
        // zeroing its color-pair table, so stale entries from pairs 8-255
        // cause corrupted/wrong colors on reload.
        const int pairLimit = std::min(COLOR_PAIRS, 256);
        for (int i = 1; i < pairLimit; ++i)
            init_pair((short)i, COLOR_WHITE, COLOR_BLACK);

        // Re-register the 7 reserved basic-color pairs.
        init_pair(1, COLOR_WHITE,   COLOR_BLACK);
        init_pair(2, COLOR_RED,     COLOR_BLACK);
        init_pair(3, COLOR_GREEN,   COLOR_BLACK);
        init_pair(4, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(5, COLOR_BLUE,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_CYAN,    COLOR_BLACK);
        _has256Colors = (COLORS >= 256 && COLOR_PAIRS > 8);
    }

    _colorPairs.clear();
    _nextPairNum = 8;

    raw();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    curs_set(0);
    bkgd(COLOR_PAIR(1));

    // Enable all mouse movement reporting (xterm protocol)
    printf("\033[?1003h");
    fflush(stdout);

    _lastGrid = {};
    _lastRelX = -1.f;
    _lastRelY = -1.f;
    _prevRelX = -1.f;
    _prevRelY = -1.f;
    _terminalWidth  = COLS  > 0 ? COLS  : NCursesConfig::DEFAULT_TERMINAL_WIDTH;
    _terminalHeight = LINES > 0 ? LINES : NCursesConfig::DEFAULT_TERMINAL_HEIGHT;

    _pendingMouseEvents.clear();

    wclear(stdscr);
    refresh();
    _isOpen = true;
}

void NCursesDisplay::close()
{
    if (_isOpen)
    {
        // Disable xterm mouse movement reporting before restoring terminal
        printf("\033[?1003l");
        fflush(stdout);
        endwin();   // saves & restores shell mode internally — no extra call needed
        _isOpen = false;
        _colorPairs.clear();
        _nextPairNum = 8;
        _pendingMouseEvents.clear();
    }
}

// ─── clear / display ──────────────────────────────────────────────────────────
void NCursesDisplay::clear()
{
    _terminalWidth  = COLS  > 0 ? COLS  : NCursesConfig::DEFAULT_TERMINAL_WIDTH;
    _terminalHeight = LINES > 0 ? LINES : NCursesConfig::DEFAULT_TERMINAL_HEIGHT;
    if (has_colors())
        bkgd(COLOR_PAIR(1));
    erase();
}

void NCursesDisplay::display()
{
    refresh();
}

// ─── Color pair registry ──────────────────────────────────────────────────────
int NCursesDisplay::getOrCreatePair(int fgIdx, int bgIdx)
{
    if (!_has256Colors)
        return 1;

    const auto key = std::make_pair(fgIdx, bgIdx);
    const auto it  = _colorPairs.find(key);
    if (it != _colorPairs.end())
        return it->second;

    if (_nextPairNum >= COLOR_PAIRS)
    {
        for (const auto& [k, v] : _colorPairs)
        {
            if (k.first == fgIdx)
                return v;
        }
        return 1;
    }

    init_pair((short)_nextPairNum, (short)fgIdx, (short)bgIdx);
    _colorPairs[key] = _nextPairNum;
    return _nextPairNum++;
}

void NCursesDisplay::resetColorPairs()
{
    _colorPairs.clear();
    _nextPairNum = 8;

    if (has_colors())
    {
        init_pair(1, COLOR_WHITE,   COLOR_BLACK);
        init_pair(2, COLOR_RED,     COLOR_BLACK);
        init_pair(3, COLOR_GREEN,   COLOR_BLACK);
        init_pair(4, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(5, COLOR_BLUE,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_CYAN,    COLOR_BLACK);
    }
}

// ─── cellAttr / scaleAttr ─────────────────────────────────────────────────────
chtype NCursesDisplay::cellAttr(const Color& fg, const Color& bg) noexcept
{
    if (!_has256Colors)
    {
        const int fgBasic = rgbToBasic(fg);
        const bool hasBg  = ((int)bg.r + (int)bg.g + (int)bg.b) >= 80;
        const int pairNum = basicColorToPair(fgBasic);
        return COLOR_PAIR(pairNum) | (hasBg ? A_REVERSE : 0);
    }
    return COLOR_PAIR(getOrCreatePair(rgbTo256(fg), rgbTo256(bg)));
}

chtype NCursesDisplay::scaleAttr(float scale) const noexcept
{
    if (scale >= 2.0f)
        return A_BOLD | A_STANDOUT;
    if (scale >= 1.5f)
        return A_BOLD;
    if (scale <= 0.7f)
        return A_DIM;
    return 0;
}

// ─── fillMouseEvent ───────────────────────────────────────────────────────────
void NCursesDisplay::fillMouseEvent(MouseEvent& mouseEvent, int termX, int termY) const
{
    mouseEvent.relativeX = std::clamp(termX / (float)std::max(1, _terminalWidth),  0.f, 1.f);
    mouseEvent.relativeY = std::clamp(termY / (float)std::max(1, _terminalHeight), 0.f, 1.f);
    mouseEvent.gridX = -1;
    mouseEvent.gridY = -1;

    if (_lastGrid.gridWidth > 0)
    {
        const int localCol = termX - _lastGrid.originCol;
        const int localRow = termY - _lastGrid.originRow;
        const int viewX = (localCol >= 0) ? localCol / std::max(1, _lastGrid.charWidth)  : -1;
        const int viewY = (localRow >= 0) ? localRow / std::max(1, _lastGrid.charHeight) : -1;

        if (viewX >= 0 && viewX < _lastGrid.viewWidth &&
            viewY >= 0 && viewY < _lastGrid.viewHeight)
        {
            mouseEvent.gridX = _lastGrid.cameraX + viewX;
            mouseEvent.gridY = _lastGrid.cameraY + viewY;
        }
    }
}

// ─── drawBlock ────────────────────────────────────────────────────────────────
void NCursesDisplay::drawBlock(int row, int col, int numRows, int numCols,
                                char c, chtype attr,
                                int clipRow0, int clipRow1)
{
    const int cx = numCols / 2;
    const int cy = numRows / 2;

    for (int dy = 0; dy < numRows; ++dy)
    {
        for (int dx = 0; dx < numCols; ++dx)
        {
            int r  = row + dy;
            int cc = col + dx;
            if (r < clipRow0 || r >= clipRow1 || cc < 0 || cc >= _terminalWidth)
                continue;
            char ch = (dx == cx && dy == cy) ? c : ' ';
            attron(attr);
            mvaddch(r, cc, (unsigned char)ch);
            attroff(attr);
        }
    }
}

// ─── drawGrid ─────────────────────────────────────────────────────────────────
void NCursesDisplay::drawGrid(const std::vector<Cell>& cells,
                               int gridWidth, int gridHeight,
                               float x1, float y1, float x2, float y2,
                               int focusX, int focusY)
{
    if (!_isOpen || gridWidth <= 0 || gridHeight <= 0 || cells.empty())
        return;

    const int bbRow1 = (int)(y1 * _terminalHeight);
    const int bbRow2 = (int)(y2 * _terminalHeight);
    const int bbCol1 = (int)(x1 * _terminalWidth);
    const int bbCol2 = (int)(x2 * _terminalWidth);
    const int areaRows = bbRow2 - bbRow1;
    const int areaCols = bbCol2 - bbCol1;

    if (areaRows <= 0 || areaCols <= 0)
        return;

    int charHeight = std::max(1, areaRows / gridHeight);
    int charWidth  = std::min(charHeight * 2, NCursesConfig::MAX_CHARS_PER_CELL);
    charHeight = std::max(1, charWidth / 2);
    charWidth  = std::max(NCursesConfig::MIN_CHARS_PER_CELL, charWidth);
    charHeight = std::max(1, charWidth / 2);

    while (charWidth > 1 && charWidth * gridWidth > areaCols)
    {
        charWidth--;
        charHeight = std::max(1, charWidth / 2);
    }

    if (charWidth * gridWidth > areaCols)
    {
        charWidth  = std::max(1, areaCols / std::max(1, gridWidth));
        charHeight = 1;
    }

    const int viewWidth  = std::min(gridWidth,  areaCols / std::max(1, charWidth));
    const int viewHeight = std::min(gridHeight, areaRows / std::max(1, charHeight));

    int cameraX = 0;
    int cameraY = 0;
    if (focusX >= 0)
        cameraX = std::clamp(focusX - viewWidth  / 2, 0, std::max(0, gridWidth  - viewWidth));
    if (focusY >= 0)
        cameraY = std::clamp(focusY - viewHeight / 2, 0, std::max(0, gridHeight - viewHeight));

    const int originCol = bbCol1 + (areaCols - viewWidth  * charWidth)  / 2;
    const int originRow = bbRow1 + (areaRows - viewHeight * charHeight) / 2;

    _lastGrid = { gridWidth, gridHeight, charWidth, charHeight, originCol, originRow, viewWidth, viewHeight, cameraX, cameraY };

    if (_has256Colors)
    {
        const Color darkBg{20, 20, 20, 255};
        const chtype bgAttr = COLOR_PAIR(getOrCreatePair(COLOR_WHITE, rgbTo256(darkBg)));
        for (int r = bbRow1; r < bbRow2; ++r)
        {
            attron(bgAttr);
            mvhline(r, bbCol1, ' ', areaCols);
            attroff(bgAttr);
        }
    }

    for (int viewY = 0; viewY < viewHeight; ++viewY)
    {
        for (int viewX = 0; viewX < viewWidth; ++viewX)
        {
            const int gridX = cameraX + viewX;
            const int gridY = cameraY + viewY;

            if (gridX >= gridWidth || gridY >= gridHeight)
                continue;

            const Cell& cell = cells[gridY * gridWidth + gridX];
            chtype attr = cellAttr(cell.fg, cell.bg);
            if (cell.highlight) attr |= A_STANDOUT;
            const int row = originRow + viewY * charHeight;
            const int col = originCol + viewX * charWidth;
            drawBlock(row, col, charHeight, charWidth, cell.c, attr, bbRow1, bbRow2);
        }
    }
}

// ─── drawText ─────────────────────────────────────────────────────────────────
void NCursesDisplay::drawText(const std::string& text, float relX, float relY,
                               const Color& fg, const Color* bg, float scale)
{
    if (!_isOpen || text.empty())
        return;

    const int row = std::clamp((int)(relY * _terminalHeight), 0, _terminalHeight - 1);
    const int col = (int)(relX * _terminalWidth) - (int)text.size() / 2;
    const int safeCol = std::max(0, col);
    const int skip    = safeCol - col;
    const int maxLen  = _terminalWidth - safeCol;

    if (maxLen <= 0 || skip >= (int)text.size())
        return;

    const Color bgColor = bg ? *bg : Color{0, 0, 0, 255};
    const chtype attr = cellAttr(fg, bgColor) | scaleAttr(scale);
    attron(attr);
    mvaddstr(row, safeCol, text.substr((size_t)skip, (size_t)maxLen).c_str());
    attroff(attr);
}

// ─── drawLine ─────────────────────────────────────────────────────────────────
void NCursesDisplay::drawLine(char c, float x1, float x2, float y,
                               const Color& fg, const Color& bg, float scale)
{
    if (!_isOpen)
        return;

    const int row  = std::clamp((int)(y  * _terminalHeight), 0, _terminalHeight - 1);
    const int col1 = std::clamp((int)(x1 * _terminalWidth),  0, _terminalWidth);
    const int col2 = std::clamp((int)(x2 * _terminalWidth),  0, _terminalWidth);

    if (col2 <= col1)
        return;

    const chtype attr = cellAttr(fg, bg) | scaleAttr(scale);
    attron(attr);
    mvhline(row, col1, (unsigned char)c, col2 - col1);
    attroff(attr);
}

// ─── getKeys ──────────────────────────────────────────────────────────────────
// Mouse events are collected here into _pendingMouseEvents,
// exactly like the SFML and SDL2 backends do.
//
// The old design had two separate problems:
//   1) getKeys() did not handle KEY_MOUSE at all → the token stayed in
//      ncurses' internal queue and corrupted the next multi-byte escape
//      sequence (F1, F2, arrow keys).
//   2) getMouseEvents() called getmouse() independently, but ncurses
//      only queues a MEVENT when getch() returns KEY_MOUSE.  Since
//      getKeys() consumed all getch() output first, getMouseEvents()
//      always found an empty queue → mouse clicks were silently lost.
//
// Now the flow matches SFML/SDL2:
//   getKeys()        → polls ALL ncurses input, stores mouse events aside
//   getMouseEvents() → returns what was collected during getKeys()
std::vector<Key> NCursesDisplay::getKeys()
{
    std::vector<Key> keys;

    _pendingMouseEvents.clear();
    _prevRelX = _lastRelX;
    _prevRelY = _lastRelY;

    int ch;
    while ((ch = getch()) != ERR)
    {
        if (ch == KEY_RESIZE)
        {
            resize_term(0, 0);
            clearok(stdscr, TRUE);
            _terminalWidth  = COLS  > 0 ? COLS  : NCursesConfig::DEFAULT_TERMINAL_WIDTH;
            _terminalHeight = LINES > 0 ? LINES : NCursesConfig::DEFAULT_TERMINAL_HEIGHT;
            continue;
        }
        if (ch == KEY_MOUSE)
        {
            MEVENT ncursesEvent;
            if (getmouse(&ncursesEvent) == OK)
            {
                MouseEvent mouseEvent;
                fillMouseEvent(mouseEvent, ncursesEvent.x, ncursesEvent.y);
                _lastRelX = mouseEvent.relativeX;
                _lastRelY = mouseEvent.relativeY;

                if      (ncursesEvent.bstate & BUTTON1_CLICKED)       mouseEvent.button = Key::MOUSE_LEFT;
                else if (ncursesEvent.bstate & BUTTON1_PRESSED)       mouseEvent.button = Key::MOUSE_LEFT;
                else if (ncursesEvent.bstate & BUTTON2_CLICKED)       mouseEvent.button = Key::MOUSE_MIDDLE;
                else if (ncursesEvent.bstate & BUTTON3_CLICKED)       mouseEvent.button = Key::MOUSE_RIGHT;
                else if (ncursesEvent.bstate & BUTTON3_PRESSED)       mouseEvent.button = Key::MOUSE_RIGHT;
                else if (ncursesEvent.bstate & REPORT_MOUSE_POSITION) mouseEvent.button = Key::NONE;

                _pendingMouseEvents.push_back(mouseEvent);
            }
            continue;
        }
        switch (ch)
        {
            case 27:        keys.push_back(Key::ESCAPE); break;
            case KEY_UP:    keys.push_back(Key::UP);     break;
            case KEY_DOWN:  keys.push_back(Key::DOWN);   break;
            case KEY_LEFT:  keys.push_back(Key::LEFT);   break;
            case KEY_RIGHT: keys.push_back(Key::RIGHT);  break;
            case '\n':      keys.push_back(Key::ENTER);  break;
            case ' ':       keys.push_back(Key::SPACE);  break;
            case '\t':      keys.push_back(Key::TAB);    break;
            case KEY_F(1):  keys.push_back(Key::F1);     break;
            case KEY_F(2):  keys.push_back(Key::F2);     break;
            case '+':           keys.push_back(Key::PLUS);      break;
            case '=':           keys.push_back(Key::PLUS);      break;
            case '-':           keys.push_back(Key::MINUS);     break;
            case KEY_BACKSPACE: keys.push_back(Key::BACKSPACE);  break;
            case 127:           keys.push_back(Key::BACKSPACE);  break;
            case '\b':          keys.push_back(Key::BACKSPACE);  break;
            default:
                if (ch >= 'a' && ch <= 'z')
                {
                    keys.push_back(static_cast<Key>(
                        ch - 'a' + static_cast<int>(Key::A)));
                    _textInput += ch;
                }
                else if (ch >= 'A' && ch <= 'Z')
                {
                    keys.push_back(static_cast<Key>(
                        ch - 'A' + static_cast<int>(Key::A)));
                    _textInput += ch;
                }
                break;
        }
    }
    return keys;
}

// ─── getMouseEvents ───────────────────────────────────────────────────────────
// Simply return what was collected during getKeys().
std::vector<MouseEvent> NCursesDisplay::getMouseEvents()
{
    return _pendingMouseEvents;
}

std::pair<float,float> NCursesDisplay::getMousePosition() const
{
    return {_lastRelX, _lastRelY};
}

bool NCursesDisplay::hasMouseMoved() const
{
    return _lastRelX != _prevRelX || _lastRelY != _prevRelY;
}

std::string NCursesDisplay::getTextInput()
{
    std::string out;
    std::swap(out, _textInput);
    return out;
}

// ─── drawRect ─────────────────────────────────────────────────────────────────
void NCursesDisplay::drawRect(float x1, float y1, float x2, float y2, const Color& color)
{
    if (!_isOpen)
        return;
    const int row1 = std::clamp((int)(y1 * _terminalHeight), 0, _terminalHeight);
    const int row2 = std::clamp((int)(y2 * _terminalHeight), 0, _terminalHeight);
    const int col1 = std::clamp((int)(x1 * _terminalWidth),  0, _terminalWidth);
    const int col2 = std::clamp((int)(x2 * _terminalWidth),  0, _terminalWidth);

    if (col2 <= col1 || row2 <= row1)
        return;

    const chtype attr = cellAttr(Colors::White, color);
    for (int r = row1; r < row2; ++r)
    {
        attron(attr);
        mvhline(r, col1, ' ', col2 - col1);
        attroff(attr);
    }
}

} // namespace Arcade

extern "C" {
    Arcade::IDisplayModule* createDisplay() { return new Arcade::NCursesDisplay(); }
    void destroyDisplay(Arcade::IDisplayModule* d) { delete d; }
}
