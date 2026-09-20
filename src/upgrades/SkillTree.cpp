#include "upgrades/SkillTree.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace chessadventure {

SkillTree SkillTree::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("SkillTree: could not open " + path);
    }

    nlohmann::json j;
    in >> j;

    SkillTree tree;
    tree.piece_id_ = j.at("pieceId").get<std::string>();

    for (const auto& node_json : j.at("nodes")) {
        UpgradeNode node;
        node.id = node_json.at("id").get<std::string>();
        node.name = node_json.value("name", node.id);
        node.description = node_json.value("description", "");
        node.cost = node_json.value("cost", 1);
        node.requires_ids = node_json.value("requires", std::vector<std::string>{});
        node.grants_skill = node_json.value("grantsSkill", "");

        if (node_json.contains("statDeltas")) {
            const auto& deltas = node_json.at("statDeltas");
            node.stat_deltas.hp = deltas.value("hp", 0);
            node.stat_deltas.attack = deltas.value("attack", 0);
            node.stat_deltas.defense = deltas.value("defense", 0);
            node.stat_deltas.move_range = deltas.value("moveRange", 0);
        }

        tree.nodes_.push_back(std::move(node));
    }

    return tree;
}

const UpgradeNode* SkillTree::FindNode(const std::string& id) const {
    auto it = std::find_if(
        nodes_.begin(), nodes_.end(), [&](const UpgradeNode& n) { return n.id == id; });
    return it != nodes_.end() ? &(*it) : nullptr;
}

bool SkillTree::CanUnlock(const std::string& node_id,
                          const std::vector<std::string>& unlocked) const {
    const UpgradeNode* node = FindNode(node_id);
    if (!node)
        return false;
    if (std::find(unlocked.begin(), unlocked.end(), node_id) != unlocked.end()) {
        return false;  // already unlocked
    }
    for (const auto& req : node->requires_ids) {
        if (std::find(unlocked.begin(), unlocked.end(), req) == unlocked.end()) {
            return false;
        }
    }
    return true;
}

}  // namespace chessadventure
