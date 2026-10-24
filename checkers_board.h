#pragma once
#include "checkers_algo.h"
#include <cstddef>

namespace checkers {
enum class SensorStatus { Accepted, Unchanged, InvalidPacket, OutOfOrder, IllegalPosition, Ambiguous };
class BoardTracker {
public:
    explicit BoardTracker(const Board& initial = Board::initial());
    const Board& board() const { return board_; }
    bool reset(const Board& board);
    bool submitMove(const Move& move);
    SensorStatus submitOccupancy(uint64_t occupied, Move* detected = nullptr);
    SensorStatus submitPacket(const uint8_t* packet, size_t size, Move* detected = nullptr);
    static uint64_t occupancy(const Board& board);
private:
    Board board_;
    bool hasSequence_ = false;
    uint8_t sequence_ = 0;
};
}
