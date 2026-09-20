#include <doctest/doctest.h>

#include <algorithm>

#include "pieces/MoveRules.h"

using namespace chessadventure;

namespace {

bool Contains(const std::vector<Position>& moves, Position p) {
    return std::find(moves.begin(), moves.end(), p) != moves.end();
}

}  // namespace

TEST_CASE("knight jump ignores blockers and stays in bounds") {
    MoveContext ctx;
    ctx.from = {0, 0};
    ctx.board_width = 8;
    ctx.board_height = 8;
    ctx.is_occupied = [](Position) { return false; };
    ctx.is_enemy = [](Position) { return false; };

    auto moves = ComputeLegalMoves(MovePattern::kKnightJump, ctx);

    // From a corner, only two of the eight knight offsets stay on the board.
    CHECK(moves.size() == 2);
    CHECK(Contains(moves, {1, 2}));
    CHECK(Contains(moves, {2, 1}));
}

TEST_CASE("rook slides until blocked by a friendly piece") {
    MoveContext ctx;
    ctx.from = {3, 3};
    ctx.board_width = 8;
    ctx.board_height = 8;
    ctx.move_range = 8;
    ctx.is_occupied = [](Position p) { return p == Position{3, 5}; };
    ctx.is_enemy = [](Position) { return false; };  // the blocker is a friendly piece

    auto moves = ComputeLegalMoves(MovePattern::kOrthogonalAny, ctx);

    CHECK(Contains(moves, {3, 4}));        // can step up to just before the blocker
    CHECK_FALSE(Contains(moves, {3, 5}));  // can't land on a friendly piece
    CHECK_FALSE(Contains(moves, {3, 6}));  // can't slide past it either
}

TEST_CASE("rook capturing an enemy stops sliding but includes that square") {
    MoveContext ctx;
    ctx.from = {3, 3};
    ctx.board_width = 8;
    ctx.board_height = 8;
    ctx.move_range = 8;
    ctx.is_occupied = [](Position p) { return p == Position{3, 5}; };
    ctx.is_enemy = [](Position p) { return p == Position{3, 5}; };

    auto moves = ComputeLegalMoves(MovePattern::kOrthogonalAny, ctx);

    CHECK(Contains(moves, {3, 5}));        // capture square included
    CHECK_FALSE(Contains(moves, {3, 6}));  // but nothing beyond it
}

TEST_CASE("pawn moves forward one and captures only diagonally") {
    MoveContext ctx;
    ctx.from = {4, 6};
    ctx.board_width = 8;
    ctx.board_height = 8;
    ctx.pawn_forward_dir = -1;  // player pawns move toward row 0
    ctx.is_occupied = [](Position p) { return p == Position{3, 5} || p == Position{4, 5}; };
    ctx.is_enemy = [](Position p) { return p == Position{3, 5}; };

    auto moves = ComputeLegalMoves(MovePattern::kPawnForward, ctx);

    CHECK_FALSE(Contains(moves, {4, 5}));  // straight ahead is blocked, even by an enemy
    CHECK(Contains(moves, {3, 5}));        // diagonal capture available
    CHECK_FALSE(Contains(moves, {5, 5}));  // no piece to capture on the other diagonal
}

TEST_CASE("king (omni_one) moves exactly one square in any direction") {
    MoveContext ctx;
    ctx.from = {4, 4};
    ctx.board_width = 8;
    ctx.board_height = 8;
    ctx.is_occupied = [](Position) { return false; };
    ctx.is_enemy = [](Position) { return false; };

    auto moves = ComputeLegalMoves(MovePattern::kOmniOne, ctx);

    CHECK(moves.size() == 8);
    CHECK_FALSE(Contains(moves, {6, 4}));  // two squares away is out of range
}
