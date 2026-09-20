#include <doctest/doctest.h>

#include "game/Board.h"
#include "pieces/Piece.h"

using namespace chessadventure;

TEST_CASE("Board rejects moves onto walls and out of bounds") {
    Board board(4, 4);
    board.SetTile({2, 2}, Tile{TileType::kWall});
    board.AddPiece(Piece("pawn", Team::kPlayer, {1, 1}, MovePattern::kOmniOne, 10, 1, 0, 1));

    CHECK_FALSE(board.MovePiece({1, 1}, {2, 2}));  // wall
    CHECK_FALSE(board.MovePiece({1, 1}, {9, 9}));  // out of bounds
    CHECK(board.MovePiece({1, 1}, {1, 2}));        // ordinary floor move
    CHECK(board.PieceAt({1, 2}) != nullptr);
    CHECK(board.PieceAt({1, 1}) == nullptr);
}

TEST_CASE("Board move onto an enemy captures it, onto an ally is rejected") {
    Board board(4, 4);
    Piece* mover = board.AddPiece(
        Piece("rook", Team::kPlayer, {0, 0}, MovePattern::kOrthogonalAny, 10, 1, 0, 8));
    board.AddPiece(Piece("pawn", Team::kEnemy, {0, 1}, MovePattern::kPawnForward, 5, 1, 0, 1));

    CHECK(board.MovePiece({0, 0}, {0, 1}));
    CHECK(board.PieceAt({0, 1}) == mover);
    CHECK(board.pieces().size() == 1);  // captured piece removed

    board.AddPiece(Piece("rook", Team::kPlayer, {3, 3}, MovePattern::kOrthogonalAny, 10, 1, 0, 8));
    CHECK_FALSE(board.MovePiece({0, 1}, {3, 3}));  // occupied by own team
}
