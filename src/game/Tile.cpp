#include "game/Tile.h"

#include <unordered_map>

namespace chessadventure {

TileType TileTypeFromString(const std::string& name) {
    static const std::unordered_map<std::string, TileType> kLookup = {
        {"floor", TileType::kFloor},
        {"wall", TileType::kWall},
        {"hazard", TileType::kHazard},
        {"player_spawn", TileType::kPlayerSpawn},
        {"enemy_spawn", TileType::kEnemySpawn},
        {"objective", TileType::kObjective},
    };
    auto it = kLookup.find(name);
    return it != kLookup.end() ? it->second : TileType::kFloor;
}

std::string TileTypeToString(TileType type) {
    switch (type) {
        case TileType::kFloor:
            return "floor";
        case TileType::kWall:
            return "wall";
        case TileType::kHazard:
            return "hazard";
        case TileType::kPlayerSpawn:
            return "player_spawn";
        case TileType::kEnemySpawn:
            return "enemy_spawn";
        case TileType::kObjective:
            return "objective";
    }
    return "floor";
}

}  // namespace chessadventure
