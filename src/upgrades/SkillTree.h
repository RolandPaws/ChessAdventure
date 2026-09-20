#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "upgrades/Upgrade.h"

namespace chessadventure {

// A loaded skill tree definition for one piece type, e.g. assets/data/upgrades/pawn.json.
// This is shared, immutable template data; which nodes a *specific* piece instance
// has unlocked lives on that Piece (see pieces/Piece.h), not here.
class SkillTree {
public:
    static SkillTree LoadFromFile(const std::string& path);

    const std::string& piece_id() const {
        return piece_id_;
    }
    const std::vector<UpgradeNode>& nodes() const {
        return nodes_;
    }

    const UpgradeNode* FindNode(const std::string& id) const;

    // A node is unlockable if it exists, isn't already unlocked, and every id in
    // its `requires_ids` is present in `unlocked`.
    bool CanUnlock(const std::string& node_id, const std::vector<std::string>& unlocked) const;

private:
    std::string piece_id_;
    std::vector<UpgradeNode> nodes_;
};

}  // namespace chessadventure
