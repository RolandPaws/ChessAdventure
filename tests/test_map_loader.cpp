#include <doctest/doctest.h>

#include <cstdio>
#include <filesystem>

#include "maps/MapLoader.h"
#include "pieces/PieceDatabase.h"

using namespace chessadventure;

namespace {
std::string AssetPath(const std::string& relative) {
    return std::string(CHESSADVENTURE_ASSETS_DIR) + "/" + relative;
}
}  // namespace

TEST_CASE("example map loads with the expected dimensions, spawns, and story") {
    MapData map = MapLoader::LoadFromFile(AssetPath("maps/example_map.json"));

    CHECK(map.width == 8);
    CHECK(map.height == 8);
    CHECK(map.tiles.size() == 64);
    CHECK(map.spawns.size() == 32);  // full classic chess set, both sides
    CHECK_FALSE(map.story.intro.empty());
    CHECK(map.story.victory_condition == "eliminate_all");
}

TEST_CASE("BuildBoardFromMap spawns every declared piece at its declared position") {
    PieceDatabase db = PieceDatabase::LoadFromFile(AssetPath("data/pieces.json"));
    MapData map = MapLoader::LoadFromFile(AssetPath("maps/example_map.json"));

    Board board = BuildBoardFromMap(map, db);

    CHECK(board.pieces().size() == 32);
    Piece* enemy_king = board.PieceAt({4, 0});
    REQUIRE(enemy_king != nullptr);
    CHECK(enemy_king->template_id() == "king");
    CHECK(enemy_king->team() == Team::kEnemy);
}

TEST_CASE("round-tripping a map through SaveToFile preserves its content") {
    MapData original = MapData::MakeEmpty(3, 2);
    original.name = "Round Trip";
    original.SetTile(1, 0, TileType::kWall);
    original.spawns.push_back({"pawn", "player", 0, 1, 1});
    original.story.intro = "test";

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "chessadventure_roundtrip_test.json").string();
    MapLoader::SaveToFile(original, temp_path);
    MapData reloaded = MapLoader::LoadFromFile(temp_path);
    std::remove(temp_path.c_str());

    CHECK(reloaded.name == original.name);
    CHECK(reloaded.width == original.width);
    CHECK(reloaded.height == original.height);
    CHECK(reloaded.TileAt(1, 0) == TileType::kWall);
    REQUIRE(reloaded.spawns.size() == 1);
    CHECK(reloaded.spawns[0].piece_id == "pawn");
    CHECK(reloaded.story.intro == "test");
}
