#include "checkers_demo.h"
#include <algorithm>
#include <chrono>
#include <windowsx.h>

namespace checkers {
namespace {
constexpr int originX = 35, originY = 65, cell = 66;
bool prefix(const Move& m, const std::vector<uint8_t>& p) {
    return m.path.size() >= p.size() && std::equal(p.begin(), p.end(), m.path.begin());
}
std::wstring notation(const Move& m) {
    std::wstring text;
    for (auto square : m.path) {
        if (!text.empty()) text += m.captures.empty() ? L"-" : L":";
        text += wchar_t(L'a' + square % 8);
        text += wchar_t(L'8' - square / 8);
    }
    return text;
}
void fill(HDC dc, RECT r, COLORREF color) {
    HBRUSH b = CreateSolidBrush(color); FillRect(dc, &r, b); DeleteObject(b);
}
}
CheckersDemo::CheckersDemo(BoardTracker& tracker, const CheckersBot& bot) : tracker_(tracker), bot_(bot) {}
int CheckersDemo::run(HINSTANCE instance, int show) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc; wc.hInstance = instance;
    wc.lpszClassName = L"CheckersDebugWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 1;
    window_ = CreateWindowW(wc.lpszClassName, L"Russian checkers - robot debugger", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
                            CW_USEDEFAULT, CW_USEDEFAULT, 840, 710, nullptr, nullptr, instance, this);
    if (!window_) return 1;
    ShowWindow(window_, show); UpdateWindow(window_);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return static_cast<int>(msg.wParam);
}
LRESULT CALLBACK CheckersDemo::windowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) {
    CheckersDemo* self = reinterpret_cast<CheckersDemo*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        self = static_cast<CheckersDemo*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        self->window_ = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    return self ? self->handle(msg, wp, lp) : DefWindowProcW(window, msg, wp, lp);
}
LRESULT CheckersDemo::handle(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(window_, GWLP_HINSTANCE));
        CreateWindowW(L"STATIC", L"You: white\nBot: black\n\nSearch depth (plies)", WS_CHILD | WS_VISIBLE,
                      590, 65, 205, 80, window_, nullptr, instance, nullptr);
        depthBox_ = CreateWindowW(L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                 590, 150, 195, 240, window_, reinterpret_cast<HMENU>(101), instance, nullptr);
        for (int i = 1; i <= 8; ++i) {
            std::wstring value = std::to_wstring(i);
            SendMessageW(depthBox_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(value.c_str()));
        }
        SendMessageW(depthBox_, CB_SETCURSEL, 3, 0);
        CreateWindowW(L"BUTTON", L"New game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      590, 195, 195, 35, window_, reinterpret_cast<HMENU>(102), instance, nullptr);
        CreateWindowW(L"STATIC", L"Click a piece, then a\nhighlighted square.\n\nFor multiple captures,\nselect every landing.\n\nK = king\n\nDepth counts complete\nturns, including captures.", WS_CHILD | WS_VISIBLE,
                      590, 255, 205, 210, window_, nullptr, instance, nullptr);
        status_ = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 35, 630, 760, 35, window_, nullptr, instance, nullptr);
        SetTimer(window_, 1, 40, nullptr);
        history_.push_back(tracker_.board()); refresh(); return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == 102 && !thinking_) restart();
        return 0;
    case WM_LBUTTONDOWN: click(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
    case WM_TIMER:
        if (thinking_ && pending_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            thinking_ = false;
            EnableWindow(GetDlgItem(window_, 102), TRUE);
            try {
                SearchResult result = pending_.get();
                if (result.hasMove && tracker_.submitMove(result.move)) finishTurn(result.move);
                else { gameOver_ = true; SetWindowTextW(status_, L"White wins."); }
            } catch (...) {
                gameOver_ = true;
                SetWindowTextW(status_, L"Search failed. Start a new game or reduce the depth.");
            }
            InvalidateRect(window_, nullptr, FALSE);
        }
        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: paint(); return 0;
    case WM_DESTROY: KillTimer(window_, 1); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(window_, msg, wp, lp);
}
void CheckersDemo::refresh() {
    legal_ = CheckersBot::legalMoves(tracker_.board());
    selection_.clear();
    if (legal_.empty()) {
        gameOver_ = true;
        SetWindowTextW(status_, tracker_.board().turn == Side::White ? L"Black wins." : L"White wins.");
    } else {
        std::wstring status = tracker_.board().turn == Side::White ? L"Your turn (white)." : L"Bot is thinking...";
        if (!legal_.front().captures.empty()) status += L" Capture is mandatory.";
        if (!lastMove_.empty()) status += L" Last: " + lastMove_;
        SetWindowTextW(status_, status.c_str());
    }
    InvalidateRect(window_, nullptr, FALSE);
}
void CheckersDemo::finishTurn(const Move& move) {
    lastMove_ = notation(move);
    Piece moved = history_.back().cells[move.path.front()];
    if (move.captures.empty() && (moved == Piece::WhiteKing || moved == Piece::BlackKing)) ++quietTurns_;
    else quietTurns_ = 0;
    history_.push_back(tracker_.board());
    refresh();
    if (gameOver_) return;
    int repeated = static_cast<int>(std::count(history_.begin(), history_.end(), tracker_.board()));
    if (repeated >= 3 || quietTurns_ >= 30) {
        gameOver_ = true; SetWindowTextW(status_, L"Draw (repetition / 15 king-only moves per side)."); return;
    }
    if (tracker_.board().turn == Side::Black) startBot();
}
void CheckersDemo::startBot() {
    int depth = static_cast<int>(SendMessageW(depthBox_, CB_GETCURSEL, 0, 0)) + 1;
    Board snapshot = tracker_.board();
    try {
        pending_ = std::async(std::launch::async, [this, snapshot, depth] { return bot_.chooseMove(snapshot, depth); });
        thinking_ = true;
        EnableWindow(GetDlgItem(window_, 102), FALSE);
    } catch (...) {
        gameOver_ = true; SetWindowTextW(status_, L"Could not start search. Start a new game.");
    }
}
void CheckersDemo::restart() {
    tracker_.reset(Board::initial()); gameOver_ = false; quietTurns_ = 0; lastMove_.clear();
    history_.clear(); history_.push_back(tracker_.board()); refresh();
}
void CheckersDemo::click(int x, int y) {
    if (thinking_ || gameOver_ || tracker_.board().turn != Side::White) return;
    if (x < originX || y < originY || x >= originX + 8 * cell || y >= originY + 8 * cell) return;
    uint8_t square = static_cast<uint8_t>((y - originY) / cell * 8 + (x - originX) / cell);
    std::vector<uint8_t> proposed = selection_; proposed.push_back(square);
    bool valid = false;
    for (const auto& m : legal_) if (prefix(m, proposed)) { valid = true; break; }
    if (!valid) {
        proposed = {square};
        for (const auto& m : legal_) if (prefix(m, proposed)) { valid = true; break; }
    }
    if (!valid) { selection_.clear(); InvalidateRect(window_, nullptr, FALSE); return; }
    selection_ = proposed;
    for (const auto& m : legal_) if (m.path == selection_) {
        Move completed = m;
        if (tracker_.submitMove(completed)) finishTurn(completed);
        return;
    }
    InvalidateRect(window_, nullptr, FALSE);
}
void CheckersDemo::paint() {
    PAINTSTRUCT ps{}; HDC target = BeginPaint(window_, &ps);
    RECT client{}; GetClientRect(window_, &client);
    HDC dc = CreateCompatibleDC(target);
    HBITMAP bitmap = CreateCompatibleBitmap(target, client.right, client.bottom);
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    fill(dc, client, RGB(245, 246, 248)); SetBkMode(dc, TRANSPARENT);
    const std::wstring title = L"Russian checkers / debug board";
    TextOutW(dc, 35, 24, title.c_str(), static_cast<int>(title.size()));
    for (int s = 0; s < 64; ++s) {
        int r = s / 8, c = s % 8;
        RECT box{originX + c * cell, originY + r * cell, originX + (c + 1) * cell, originY + (r + 1) * cell};
        COLORREF color = (r + c) % 2 ? RGB(125, 90, 68) : RGB(233, 216, 186);
        if (std::find(selection_.begin(), selection_.end(), s) != selection_.end()) color = RGB(208, 172, 54);
        bool destination = false;
        if (!selection_.empty()) for (const auto& m : legal_)
            if (prefix(m, selection_) && m.path.size() > selection_.size() && m.path[selection_.size()] == s) destination = true;
        if (destination) color = RGB(111, 168, 105);
        fill(dc, box, color);
        Piece p = tracker_.board().cells[s];
        if (p != Piece::Empty) {
            HBRUSH brush = CreateSolidBrush(owner(p) > 0 ? RGB(250, 248, 240) : RGB(38, 43, 53));
            HGDIOBJ old = SelectObject(dc, brush);
            Ellipse(dc, box.left + 9, box.top + 9, box.right - 9, box.bottom - 9);
            SelectObject(dc, old); DeleteObject(brush);
            if (p == Piece::WhiteKing || p == Piece::BlackKing) {
                SetTextColor(dc, owner(p) > 0 ? RGB(30, 30, 30) : RGB(255, 255, 255));
                RECT label = box; DrawTextW(dc, L"K", 1, &label, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
    SetTextColor(dc, RGB(30, 30, 30));
    for (int i = 0; i < 8; ++i) {
        wchar_t file = wchar_t(L'a' + i), rank = wchar_t(L'8' - i);
        TextOutW(dc, originX + i * cell + 29, originY + 8 * cell + 4, &file, 1);
        TextOutW(dc, 16, originY + i * cell + 24, &rank, 1);
    }
    BitBlt(target, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, oldBitmap); DeleteObject(bitmap); DeleteDC(dc); EndPaint(window_, &ps);
}
}
