#include "pieces/Piece.h"

#include <algorithm>

namespace chessadventure {

Piece::Piece(std::string template_id,
             Team team,
             Position position,
             MovePattern pattern,
             int max_hp,
             int attack,
             int defense,
             int move_range)
    : template_id_(std::move(template_id)),
      team_(team),
      position_(position),
      pattern_(pattern),
      current_hp_(max_hp),
      max_hp_(max_hp),
      attack_(attack),
      defense_(defense),
      move_range_(move_range) {}

void Piece::TakeDamage(int amount) {
    const int mitigated = std::max(0, amount - defense_);
    current_hp_ = std::max(0, current_hp_ - mitigated);
}

void Piece::GainLevel(int skill_points_awarded) {
    ++level_;
    skill_points_ += skill_points_awarded;
}

bool Piece::HasUnlocked(const std::string& node_id) const {
    return std::find(unlocked_upgrades_.begin(), unlocked_upgrades_.end(), node_id) !=
           unlocked_upgrades_.end();
}

void Piece::ApplyUpgrade(const UpgradeNode& node) {
    if (HasUnlocked(node.id))
        return;

    max_hp_ += node.stat_deltas.hp;
    current_hp_ += node.stat_deltas.hp;
    attack_ += node.stat_deltas.attack;
    defense_ += node.stat_deltas.defense;
    move_range_ += node.stat_deltas.move_range;

    if (!node.grants_skill.empty()) {
        skills_.push_back(node.grants_skill);
    }

    skill_points_ = std::max(0, skill_points_ - node.cost);
    unlocked_upgrades_.push_back(node.id);
}

}  // namespace chessadventure
