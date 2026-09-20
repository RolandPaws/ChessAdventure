#pragma once

#include <memory>
#include <vector>

#include "game/Position.h"
#include "game/Tile.h"
#include "pieces/Piece.h"

namespace chessadventure {

// Owns the grid of tiles and the live pieces placed on it for one map/battle.
// Pieces are stored by pointer so their identity survives moves/removal and so
// callers (rendering, the map editor's preview, AI) can hold stable references.
class Board {
public:
    Board(int width, int height);

    int width() const {
        return width_;
    }
    int height() const {
        return height_;
    }

    bool IsInside(Position pos) const;
    const Tile& TileAt(Position pos) const;
    void SetTile(Position pos, Tile tile);

    Piece* PieceAt(Position pos) const;
    Piece* AddPiece(Piece piece);
    void RemovePieceAt(Position pos);
    bool MovePiece(Position from, Position to);

    const std::vector<std::unique_ptr<Piece>>& pieces() const {
        return pieces_;
    }

private:
    int width_;
    int height_;
    std::vector<Tile> tiles_;  // row-major, size width_ * height_
    std::vector<std::unique_ptr<Piece>> pieces_;

    int Index(Position pos) const {
        return pos.y * width_ + pos.x;
    }
};

}  // namespace chessadventure
