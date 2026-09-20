#pragma once

#include <string>
#include <unordered_map>

#include "pieces/MoveRules.h"
#include "pieces/Piece.h"

namespace chessadventure {

// Base stat template for a piece type, as declared in assets/data/pieces.json.
// Spawning a Piece instance (PieceDatabase::Spawn) copies these values as the
// instance's starting stats; upgrades and leveling then diverge per-instance.
struct PieceTemplate {
    std::string id;
    std::string name;
    std::string description;
    MovePattern move_pattern = MovePattern::kOmniOne;
    int base_hp = 10;
    int base_attack = 1;
    int base_defense = 0;
    int base_move_range = 1;
    std::string
        skill_tree_path;  // relative to the maps/data root, e.g. assets/data/upgrades/pawn.json
};

// Loads and owns all piece templates declared in assets/data/pieces.json. This is
// read-only reference data shared by the game, the map editor, and tests.
class PieceDatabase {
public:
    static PieceDatabase LoadFromFile(const std::string& path);

    const PieceTemplate* Find(const std::string& id) const;
    const std::unordered_map<std::string, PieceTemplate>& templates() const {
        return templates_;
    }

    // Convenience: builds a live Piece instance from a known template id.
    Piece Spawn(const std::string& template_id, Team team, Position position) const;

private:
    std::unordered_map<std::string, PieceTemplate> templates_;
};

}  // namespace chessadventure
