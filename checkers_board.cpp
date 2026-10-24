#include "checkers_board.h"

namespace checkers {
BoardTracker::BoardTracker(const Board& b) : board_(Board::initial()) { reset(b); }
bool BoardTracker::reset(const Board& b) {
    if (b.turn != Side::White && b.turn != Side::Black) return false;
    int white = 0, black = 0;
    for (int s = 0; s < 64; ++s) {
        const int p = static_cast<int>(b.cells[s]);
        if (p < -2 || p > 2 || (p && (s / 8 + s % 8) % 2 == 0)) return false;
        if ((b.cells[s] == Piece::White && s / 8 == 0) ||
            (b.cells[s] == Piece::Black && s / 8 == 7)) return false;
        white += p > 0; black += p < 0;
    }
    if (white > 12 || black > 12) return false;
    board_ = b; hasSequence_ = false;
    return true;
}
bool BoardTracker::submitMove(const Move& m) {
    Board next;
    if (!CheckersBot::applyMove(board_, m, next)) return false;
    board_ = next;
    return true;
}
uint64_t BoardTracker::occupancy(const Board& b) {
    uint64_t bits = 0;
    for (int i = 0; i < 64; ++i) if (b.cells[i] != Piece::Empty) bits |= uint64_t(1) << i;
    return bits;
}
SensorStatus BoardTracker::submitOccupancy(uint64_t bits, Move* detected) {
    if (bits == occupancy(board_)) return SensorStatus::Unchanged;
    bool found = false;
    Board candidate;
    Move move;
    for (const auto& m : CheckersBot::legalMoves(board_)) {
        Board next;
        CheckersBot::applyMove(board_, m, next);
        if (occupancy(next) != bits) continue;
        // A robot needs an unambiguous path, even if two paths yield the same board.
        if (found) return SensorStatus::Ambiguous;
        found = true; candidate = next; move = m;
    }
    if (!found) return SensorStatus::IllegalPosition;
    board_ = candidate;
    if (detected) *detected = move;
    return SensorStatus::Accepted;
}
SensorStatus BoardTracker::submitPacket(const uint8_t* p, size_t size, Move* detected) {
    if (!p || size != 12 || p[0] != 0xA5 || p[1] != 1) return SensorStatus::InvalidPacket;
    uint8_t checksum = 0;
    for (size_t i = 0; i < 11; ++i) checksum ^= p[i];
    if (checksum != p[11]) return SensorStatus::InvalidPacket;
    if (hasSequence_) {
        uint8_t delta = static_cast<uint8_t>(p[2] - sequence_);
        if (delta == 0 || delta >= 128) return SensorStatus::OutOfOrder;
    }
    uint64_t bits = 0;
    for (int i = 0; i < 8; ++i) bits |= uint64_t(p[3 + i]) << (8 * i);
    SensorStatus status = submitOccupancy(bits, detected);
    if (status == SensorStatus::Accepted || status == SensorStatus::Unchanged) {
        hasSequence_ = true; sequence_ = p[2];
    }
    return status;
}
}
