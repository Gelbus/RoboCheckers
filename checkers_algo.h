#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace checkers {
enum class Piece : int8_t { Empty = 0, White = 1, WhiteKing = 2, Black = -1, BlackKing = -2 };
enum class Side : int8_t { White = 1, Black = -1 };
Side opposite(Side side);
int owner(Piece piece);
struct Board {
    std::array<Piece, 64> cells{};
    Side turn = Side::White;
    static Board initial();
    bool operator==(const Board& other) const;
};
struct Move {
    std::vector<uint8_t> path;     // Start followed by every landing square.
    std::vector<uint8_t> captures; // Same order as physical captures.
    bool operator==(const Move& other) const;
};
struct SearchResult {
    bool hasMove = false;
    Move move;
    int score = 0;
    uint64_t nodes = 0;
};
class CheckersBot {
public:
    static std::vector<Move> legalMoves(const Board& board);
    static bool applyMove(const Board& board, const Move& move, Board& result);
    SearchResult chooseMove(const Board& board, int depth) const;
private:
    static int evaluate(const Board& board);
    static int search(const Board& board, int depth, int alpha, int beta,
                      int ply, uint64_t& nodes);
};
} // namespace checkers
