#pragma once

#include <functional>
#include <string>
#include <vector>

#include "game/Position.h"

namespace chessadventure {

// Movement archetypes a piece template can declare in assets/data/pieces.json.
// RPG upgrades extend a piece's stats and skills (see upgrades/SkillTree.h) but,
// in this skeleton, do not yet alter the base movement pattern itself.
enum class MovePattern {
    kOrthogonalAny,  // rook-like: unlimited straight lines
    kDiagonalAny,    // bishop-like: unlimited diagonals
    kOmniAny,        // queen-like: unlimited straight + diagonal
    kOmniOne,        // king-like: one square any direction
    kKnightJump,     // knight-like: L-shaped jump, ignores blockers in between
    kPawnForward,    // pawn-like: forward step (+ diagonal capture), team-relative
};

MovePattern MovePatternFromString(const std::string& name);

// Returns true if `pos` is inside a board of the given size.
bool IsInBounds(Position pos, int board_width, int board_height);

// `is_occupied` reports whether a square currently holds any piece.
// `is_enemy` reports whether a square holds a piece belonging to the opposing team
// (only consulted for squares where `is_occupied` is true).
using OccupancyQuery = std::function<bool(Position)>;

struct MoveContext {
    Position from;
    int board_width = 8;
    int board_height = 8;
    int move_range =
        8;  // how many steps a sliding piece may take; kOmniOne/kKnightJump ignore this
    int pawn_forward_dir = -1;  // -1 moves up the board (toward row 0), +1 moves down
    OccupancyQuery is_occupied;
    OccupancyQuery is_enemy;
};

// Computes the set of legal destination squares for a piece with the given move
// pattern, respecting board bounds, blocking pieces, and simple capture rules.
// This is pure/stateless so it can be unit tested and reused by the map editor
// for move-preview without depending on the live game loop.
std::vector<Position> ComputeLegalMoves(MovePattern pattern, const MoveContext& ctx);

}  // namespace chessadventure
