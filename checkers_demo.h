#pragma once
#include "checkers_board.h"
#include <future>
#include <string>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace checkers {
class CheckersDemo {
public:
    CheckersDemo(BoardTracker& tracker, const CheckersBot& bot);
    int run(HINSTANCE instance, int show);
private:
    BoardTracker& tracker_;
    const CheckersBot& bot_;
    HWND window_ = nullptr, depthBox_ = nullptr, status_ = nullptr;
    std::vector<uint8_t> selection_;
    std::vector<Move> legal_;
    std::future<SearchResult> pending_;
    bool thinking_ = false, gameOver_ = false;
    unsigned quietTurns_ = 0;
    std::vector<Board> history_;
    std::wstring lastMove_;
    static LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT handle(UINT, WPARAM, LPARAM);
    void paint();
    void click(int x, int y);
    void refresh();
    void startBot();
    void finishTurn(const Move& move);
    void restart();
};
}
