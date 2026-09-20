#include "pieces/PieceDatabase.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace chessadventure {

PieceDatabase PieceDatabase::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("PieceDatabase: could not open " + path);
    }

    nlohmann::json j;
    in >> j;

    PieceDatabase db;
    for (const auto& piece_json : j.at("pieces")) {
        PieceTemplate tmpl;
        tmpl.id = piece_json.at("id").get<std::string>();
        tmpl.name = piece_json.value("name", tmpl.id);
        tmpl.description = piece_json.value("description", "");
        tmpl.move_pattern = MovePatternFromString(piece_json.value("movePattern", "omni_one"));
        tmpl.skill_tree_path = piece_json.value("skillTree", "");

        const auto& stats = piece_json.at("baseStats");
        tmpl.base_hp = stats.value("hp", 10);
        tmpl.base_attack = stats.value("attack", 1);
        tmpl.base_defense = stats.value("defense", 0);
        tmpl.base_move_range = stats.value("moveRange", 1);

        db.templates_.emplace(tmpl.id, std::move(tmpl));
    }

    return db;
}

const PieceTemplate* PieceDatabase::Find(const std::string& id) const {
    auto it = templates_.find(id);
    return it != templates_.end() ? &it->second : nullptr;
}

Piece PieceDatabase::Spawn(const std::string& template_id, Team team, Position position) const {
    const PieceTemplate* tmpl = Find(template_id);
    if (!tmpl) {
        throw std::runtime_error("PieceDatabase: unknown piece template '" + template_id + "'");
    }
    return Piece(tmpl->id,
                 team,
                 position,
                 tmpl->move_pattern,
                 tmpl->base_hp,
                 tmpl->base_attack,
                 tmpl->base_defense,
                 tmpl->base_move_range);
}

}  // namespace chessadventure
