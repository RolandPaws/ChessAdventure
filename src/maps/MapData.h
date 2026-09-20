#pragma once

#include <string>
#include <vector>

#include "game/Tile.h"

namespace chessadventure {

struct PieceSpawn {
    std::string piece_id;
    std::string team;  // "player" | "enemy" — kept as string here, resolved when spawning
    int x = 0;
    int y = 0;
    int level = 1;
};

// Narrative metadata for a map. Deliberately minimal for the skeleton: enough to
// show intro/victory text and branch on a win condition. A real dialogue/quest
// system would extend this rather than replace it.
struct MapStory {
    std::string intro;
    std::string victory_condition = "eliminate_all";  // "eliminate_all" | "reach_objective"
    std::string on_victory;
};

// The full, engine-agnostic description of one map: plain data, serializable to/from
// JSON (see maps/MapLoader.h). Both the game and the visual map editor read and
// write this same struct, so a map made in the editor loads directly in-game.
struct MapData {
    std::string name = "Untitled Map";
    int width = 8;
    int height = 8;
    std::vector<TileType> tiles;  // row-major, size width * height
    std::vector<PieceSpawn> spawns;
    MapStory story;

    static MapData MakeEmpty(int width, int height);

    TileType TileAt(int x, int y) const {
        return tiles[static_cast<size_t>(y) * width + x];
    }
    void SetTile(int x, int y, TileType type) {
        tiles[static_cast<size_t>(y) * width + x] = type;
    }
};

}  // namespace chessadventure
