#pragma once

#include <string>
#include <vector>

#include "game/Position.h"
#include "pieces/MoveRules.h"
#include "upgrades/Upgrade.h"

namespace chessadventure {

enum class Team {
    kPlayer,
    kEnemy,
};

// A concrete, ownable unit on the board. `template_id` names the entry in
// assets/data/pieces.json this was spawned from; everything else here is per-instance
// state that upgrades, leveling, and combat mutate over the course of a run.
class Piece {
public:
    Piece(std::string template_id,
          Team team,
          Position position,
          MovePattern pattern,
          int max_hp,
          int attack,
          int defense,
          int move_range);

    const std::string& template_id() const {
        return template_id_;
    }
    Team team() const {
        return team_;
    }
    Position position() const {
        return position_;
    }
    void set_position(Position pos) {
        position_ = pos;
    }

    MovePattern move_pattern() const {
        return pattern_;
    }
    int level() const {
        return level_;
    }
    int skill_points() const {
        return skill_points_;
    }

    int current_hp() const {
        return current_hp_;
    }
    int max_hp() const {
        return max_hp_;
    }
    int attack() const {
        return attack_;
    }
    int defense() const {
        return defense_;
    }
    int move_range() const {
        return move_range_;
    }

    bool IsAlive() const {
        return current_hp_ > 0;
    }
    void TakeDamage(int amount);

    void GainLevel(int skill_points_awarded = 1);

    const std::vector<std::string>& unlocked_upgrades() const {
        return unlocked_upgrades_;
    }
    bool HasUnlocked(const std::string& node_id) const;

    // Applies a skill-tree node's stat deltas / granted skill and spends the
    // matching skill points. Caller (typically SkillTree::CanUnlock) is
    // responsible for prerequisite validation.
    void ApplyUpgrade(const UpgradeNode& node);

    const std::vector<std::string>& skills() const {
        return skills_;
    }

private:
    std::string template_id_;
    Team team_;
    Position position_;
    MovePattern pattern_;

    int level_ = 1;
    int skill_points_ = 0;

    int current_hp_;
    int max_hp_;
    int attack_;
    int defense_;
    int move_range_;

    std::vector<std::string> unlocked_upgrades_;
    std::vector<std::string> skills_;
};

}  // namespace chessadventure
