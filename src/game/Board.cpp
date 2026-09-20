#include "game/Board.h"

#include <algorithm>
#include <stdexcept>

namespace chessadventure {

Board::Board(int width, int height)
    : width_(width), height_(height), tiles_(static_cast<size_t>(width) * height) {}

bool Board::IsInside(Position pos) const {
    return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
}

const Tile& Board::TileAt(Position pos) const {
    if (!IsInside(pos)) {
        throw std::out_of_range("Board::TileAt: position out of bounds");
    }
    return tiles_[Index(pos)];
}

void Board::SetTile(Position pos, Tile tile) {
    if (!IsInside(pos)) {
        throw std::out_of_range("Board::SetTile: position out of bounds");
    }
    tiles_[Index(pos)] = tile;
}

Piece* Board::PieceAt(Position pos) const {
    for (const auto& piece : pieces_) {
        if (piece->position() == pos)
            return piece.get();
    }
    return nullptr;
}

Piece* Board::AddPiece(Piece piece) {
    pieces_.push_back(std::make_unique<Piece>(std::move(piece)));
    return pieces_.back().get();
}

void Board::RemovePieceAt(Position pos) {
    pieces_.erase(
        std::remove_if(pieces_.begin(),
                       pieces_.end(),
                       [&](const std::unique_ptr<Piece>& p) { return p->position() == pos; }),
        pieces_.end());
}

bool Board::MovePiece(Position from, Position to) {
    if (!IsInside(to) || !TileAt(to).IsWalkable())
        return false;
    Piece* piece = PieceAt(from);
    if (!piece)
        return false;

    Piece* target = PieceAt(to);
    if (target) {
        if (target->team() == piece->team())
            return false;  // can't capture own team
        RemovePieceAt(to);
    }

    piece->set_position(to);
    return true;
}

}  // namespace chessadventure
