#include "checkers_algo.h"
#include <algorithm>
#include <cstdlib>

namespace checkers {
Side opposite(Side s) { return s == Side::White ? Side::Black : Side::White; }
int owner(Piece p) { return (static_cast<int>(p) > 0) - (static_cast<int>(p) < 0); }
bool Board::operator==(const Board& b) const { return turn == b.turn && cells == b.cells; }
bool Move::operator==(const Move& m) const { return path == m.path && captures == m.captures; }
Board Board::initial() {
    Board b;
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c)
        if ((r + c) % 2) b.cells[r * 8 + c] = r < 3 ? Piece::Black : r > 4 ? Piece::White : Piece::Empty;
    return b;
}

namespace {
bool inside(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

bool king(Piece p) { return std::abs(static_cast<int>(p)) == 2; }

Piece promote(Piece p, int row) {
    if (p == Piece::White && row == 0) return Piece::WhiteKing;
    if (p == Piece::Black && row == 7) return Piece::BlackKing;
    return p;
}

// Captured pieces stay on the board until the complete turn ends (Russian rules).
void captures(const Board& b, int square, Piece piece, uint64_t taken,
              Move& move, std::vector<Move>& out) {
    bool extended = false;
    const int row = square / 8, col = square % 8;
    for (int dr : {-1, 1}) for (int dc : {-1, 1}) {
        int r = row + dr, c = col + dc;
        if (king(piece)) {
            while (inside(r, c) && b.cells[r * 8 + c] == Piece::Empty) { r += dr; c += dc; }
        }
        if (!inside(r, c)) continue;
        int victim = r * 8 + c;
        if (owner(b.cells[victim]) != -owner(piece) || (taken & (uint64_t(1) << victim))) continue;
        r += dr; c += dc;
        while (inside(r, c) && b.cells[r * 8 + c] == Piece::Empty) {
            const int landing = r * 8 + c;
            Board next = b;
            next.cells[square] = Piece::Empty;
            next.cells[landing] = promote(piece, r);
            move.path.push_back(static_cast<uint8_t>(landing));
            move.captures.push_back(static_cast<uint8_t>(victim));
            captures(next, landing, next.cells[landing], taken | (uint64_t(1) << victim), move, out);
            move.captures.pop_back(); move.path.pop_back();
            extended = true;
            if (!king(piece)) break;
            r += dr; c += dc;
        }
    }
    if (!extended && !move.captures.empty()) out.push_back(move);
}
Board applyUnchecked(const Board& b, const Move& m) {
    Board out = b;
    Piece p = out.cells[m.path.front()];
    out.cells[m.path.front()] = Piece::Empty;
    for (size_t i = 1; i < m.path.size(); ++i) p = promote(p, m.path[i] / 8);
    for (auto s : m.captures) out.cells[s] = Piece::Empty;
    out.cells[m.path.back()] = p;
    out.turn = opposite(b.turn);
    return out;
}
}
std::vector<Move> CheckersBot::legalMoves(const Board& b) {
    std::vector<Move> moves;
    for (int s = 0; s < 64; ++s) if (owner(b.cells[s]) == static_cast<int>(b.turn)) {
        Move m; m.path.push_back(static_cast<uint8_t>(s));
        captures(b, s, b.cells[s], 0, m, moves);
    }
    if (!moves.empty()) return moves;
    for (int s = 0; s < 64; ++s) if (owner(b.cells[s]) == static_cast<int>(b.turn)) {
        Piece p = b.cells[s];
        for (int dr : {-1, 1}) for (int dc : {-1, 1}) {
            if (!king(p) && dr != -owner(p)) continue;
            int r = s / 8 + dr, c = s % 8 + dc;
            while (inside(r, c) && b.cells[r * 8 + c] == Piece::Empty) {
                Move m; m.path = {static_cast<uint8_t>(s), static_cast<uint8_t>(r * 8 + c)};
                moves.push_back(m);
                if (!king(p)) break;
                r += dr; c += dc;
            }
        }
    }
    return moves;
}
bool CheckersBot::applyMove(const Board& b, const Move& m, Board& out) {
    auto moves = legalMoves(b);
    if (std::find(moves.begin(), moves.end(), m) == moves.end()) return false;
    out = applyUnchecked(b, m);
    return true;
}
int CheckersBot::evaluate(const Board& b) {
    int score = 0;
    for (int s = 0; s < 64; ++s) {
        Piece p = b.cells[s];
        if (p == Piece::Empty) continue;
        int advance = owner(p) > 0 ? 7 - s / 8 : s / 8;
        int center = 7 - std::abs(2 * (s % 8) - 7);
        score += owner(p) * (king(p) ? 320 + center : 100 + advance * 5 + center);
    }
    return score * static_cast<int>(b.turn);
}
int CheckersBot::search(const Board& b, int depth, int alpha, int beta, int ply, uint64_t& nodes) {
    ++nodes;
    auto moves = legalMoves(b);
    if (moves.empty()) return -100000 + ply;
    if (depth <= 0) return evaluate(b);
    for (const auto& m : moves) {
        int score = -search(applyUnchecked(b, m), depth - 1, -beta, -alpha, ply + 1, nodes);
        alpha = std::max(alpha, score);
        if (alpha >= beta) break;
    }
    return alpha;
}
SearchResult CheckersBot::chooseMove(const Board& b, int depth) const {
    SearchResult result;
    result.score = -100001;
    depth = std::max(1, std::min(depth, 10));
    for (const auto& m : legalMoves(b)) {
        int score = -search(applyUnchecked(b, m), depth - 1, -100001, -result.score, 1, result.nodes);
        if (!result.hasMove || score > result.score) {
            result.hasMove = true; result.move = m; result.score = score;
        }
    }
    return result;
}
} // namespace checkers
