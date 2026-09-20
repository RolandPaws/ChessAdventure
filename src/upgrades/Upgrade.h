#pragma once

#include <string>
#include <vector>

namespace chessadventure {

// Flat stat bonuses an upgrade node grants. Zero-valued fields are no-ops; kept as
// plain ints (not optional<int>) since every upgrade in practice touches most of
// these and the JSON loader can just default-zero missing keys.
struct StatDeltas {
    int hp = 0;
    int attack = 0;
    int defense = 0;
    int move_range = 0;
};

// A single node in a piece's skill tree (assets/data/upgrades/<piece_id>.json).
// Trees are DAGs: a node unlocks once all of its `requires` ids are unlocked on
// that piece instance, and costs `cost` skill points to claim.
struct UpgradeNode {
    std::string id;
    std::string name;
    std::string description;
    int cost = 1;
    std::vector<std::string> requires_ids;
    StatDeltas stat_deltas;
    std::string grants_skill;  // empty if this node is a pure stat boost
};

}  // namespace chessadventure
