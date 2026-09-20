#include "maps/MapLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace chessadventure {

MapData MapLoader::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("MapLoader: could not open " + path);
    }

    nlohmann::json j;
    in >> j;

    MapData data;
    data.name = j.value("name", std::string("Untitled Map"));
    data.width = j.at("width").get<int>();
    data.height = j.at("height").get<int>();

    data.tiles.reserve(static_cast<size_t>(data.width) * data.height);
    for (const auto& row : j.at("tiles")) {
        for (const auto& cell : row) {
            data.tiles.push_back(TileTypeFromString(cell.get<std::string>()));
        }
    }
    if (data.tiles.size() != static_cast<size_t>(data.width) * data.height) {
        throw std::runtime_error("MapLoader: tiles size does not match width*height in " + path);
    }

    if (j.contains("spawns")) {
        for (const auto& spawn_json : j.at("spawns")) {
            PieceSpawn spawn;
            spawn.piece_id = spawn_json.at("pieceId").get<std::string>();
            spawn.team = spawn_json.value("team", std::string("player"));
            spawn.x = spawn_json.at("x").get<int>();
            spawn.y = spawn_json.at("y").get<int>();
            spawn.level = spawn_json.value("level", 1);
            data.spawns.push_back(std::move(spawn));
        }
    }

    if (j.contains("story")) {
        const auto& story_json = j.at("story");
        data.story.intro = story_json.value("intro", "");
        data.story.victory_condition =
            story_json.value("victoryCondition", std::string("eliminate_all"));
        data.story.on_victory = story_json.value("onVictory", "");
    }

    return data;
}

void MapLoader::SaveToFile(const MapData& data, const std::string& path) {
    nlohmann::json j;
    j["name"] = data.name;
    j["width"] = data.width;
    j["height"] = data.height;

    auto tiles_json = nlohmann::json::array();
    for (int y = 0; y < data.height; ++y) {
        auto row = nlohmann::json::array();
        for (int x = 0; x < data.width; ++x) {
            row.push_back(TileTypeToString(data.TileAt(x, y)));
        }
        tiles_json.push_back(std::move(row));
    }
    j["tiles"] = std::move(tiles_json);

    auto spawns_json = nlohmann::json::array();
    for (const auto& spawn : data.spawns) {
        spawns_json.push_back({
            {"pieceId", spawn.piece_id},
            {"team", spawn.team},
            {"x", spawn.x},
            {"y", spawn.y},
            {"level", spawn.level},
        });
    }
    j["spawns"] = std::move(spawns_json);

    j["story"] = {
        {"intro", data.story.intro},
        {"victoryCondition", data.story.victory_condition},
        {"onVictory", data.story.on_victory},
    };

    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("MapLoader: could not write " + path);
    }
    out << j.dump(2);
}

Board BuildBoardFromMap(const MapData& data, const PieceDatabase& piece_db) {
    Board board(data.width, data.height);
    for (int y = 0; y < data.height; ++y) {
        for (int x = 0; x < data.width; ++x) {
            board.SetTile({x, y}, Tile{data.TileAt(x, y)});
        }
    }

    for (const auto& spawn : data.spawns) {
        const Team team = spawn.team == "enemy" ? Team::kEnemy : Team::kPlayer;
        Piece piece = piece_db.Spawn(spawn.piece_id, team, {spawn.x, spawn.y});
        for (int i = 1; i < spawn.level; ++i) {
            piece.GainLevel();
        }
        board.AddPiece(std::move(piece));
    }

    return board;
}

}  // namespace chessadventure
