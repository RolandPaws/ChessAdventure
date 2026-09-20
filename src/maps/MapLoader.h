#pragma once

#include <string>

#include "game/Board.h"
#include "maps/MapData.h"
#include "pieces/PieceDatabase.h"

namespace chessadventure {

class MapLoader {
public:
    static MapData LoadFromFile(const std::string& path);
    static void SaveToFile(const MapData& data, const std::string& path);
};

// Materializes a playable Board from map data + the piece template database,
// spawning each declared PieceSpawn as a live Piece. Shared by the game runtime
// and by the map editor's "playtest" preview so both see identical results.
Board BuildBoardFromMap(const MapData& data, const PieceDatabase& piece_db);

}  // namespace chessadventure
