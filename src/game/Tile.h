#pragma once

#include <string>

namespace chessadventure {

// The terrain/role a single board square can have. Custom maps assign one of these
// per tile; new kinds should stay coarse (visual + rules meaning), with anything
// map-specific (story triggers, dialogue) layered on top via MapData, not here.
enum class TileType {
    kFloor,
    kWall,
    kHazard,
    kPlayerSpawn,
    kEnemySpawn,
    kObjective,
};

TileType TileTypeFromString(const std::string& name);
std::string TileTypeToString(TileType type);

struct Tile {
    TileType type = TileType::kFloor;

    bool IsWalkable() const {
        return type != TileType::kWall;
    }
};

}  // namespace chessadventure
